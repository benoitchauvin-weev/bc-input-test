#include "app/App.h"

#include "core/Params.h"
#include "input/Input.h"
#include "render/Renderer.h"
#include "test/FrameRateProbe.h"
#include "test/InputTest.h"
#include "window/Window.h"

#include <cstdio>

#define WIN32_LEAN_AND_MEAN (1)
#define NOMINMAX (1)
#include <windows.h>
#include <dwmapi.h>

namespace weev
{

int WvApp::Run(const int nArgCount, char** const ppszArgs)
{
	WvParams::Init(nArgCount, ppszArgs);

	if (!WvTimer::Init())
	{
		std::fprintf(stderr, "no performance counter\n");
		return 1;
	}

	// A self-test replaces the app, as in the engine: nothing else is
	// initialised, and no window exists.
	if (WvParams::Has("test"))
	{
		return WvInputTest::Run() ? 0 : 1;
	}

	if (WvParams::Has("probe_frame_rate"))
	{
		WvFrameRateProbe::Run();
		return 0;
	}

	static WvApp s_oApp;

	if (!s_oApp.Init())
	{
		s_oApp.Shutdown();
		return 1;
	}

	const int nExitCode = s_oApp.Loop();
	s_oApp.Shutdown();
	return nExitCode;
}

bool WvApp::Init()
{
	m_uqwFrameLimit = uint64_t(WvParams::GetInt("frames", 0));
	m_bHeadless = WvParams::Has("headless");
	m_bAgent = WvParams::Has("agent");

	if (WvParams::Has("fixed_dt"))
	{
		m_eMode = Mode::MODE_FIXED;
		m_uqwFixedStepMicroseconds =
			uint64_t(WvParams::GetInt("fixed_dt", 16667));
	}

	const char* const pszReplayPath = WvParams::GetString("replay", nullptr);

	if (pszReplayPath != nullptr)
	{
		if (!m_oRecording.Load(pszReplayPath))
		{
			std::fprintf(stderr, "cannot load recording %s\n", pszReplayPath);
			return false;
		}

		m_eMode = Mode::MODE_REPLAY;

		// The recording already holds the agent's events; scheduling
		// them again would press every key twice.
		m_bAgent = false;

		std::printf(
			"replaying %u frames from %s\n",
			m_oRecording.GetFrameCount(),
			pszReplayPath
		);
	}

	m_pszRecordPath = WvParams::GetString("record", nullptr);
	m_bRecording = m_pszRecordPath != nullptr && m_eMode != Mode::MODE_REPLAY;

	if (m_bHeadless && m_eMode == Mode::MODE_LIVE)
	{
		std::fprintf(stderr, "--headless needs --fixed_dt or --replay\n");
		return false;
	}

	if (!WvInput::Init())
	{
		std::fprintf(stderr, "input init failed\n");
		return false;
	}

	if (!m_bHeadless)
	{
		if (!WvWindow::Create(L"bc-input-test", 1280, 720) ||
			!WvRenderer::Init(WvWindow::GetNativeHandle()))
		{
			std::fprintf(stderr, "window init failed\n");
			return false;
		}

		m_bWindow = true;
	}

	m_oGame.Init();
	m_oInputBuilder.Reset();
	m_oTimer.Reset();
	return true;
}

void WvApp::Shutdown()
{
	if (m_bRecording)
	{
		if (m_oRecording.Save(m_pszRecordPath))
		{
			std::printf(
				"recorded %u frames to %s\n",
				m_oRecording.GetFrameCount(),
				m_pszRecordPath
			);
		}
		else
		{
			std::fprintf(stderr, "cannot save %s\n", m_pszRecordPath);
		}
	}

	if (m_bWindow)
	{
		WvRenderer::Shutdown();
		WvWindow::Shutdown();
		m_bWindow = false;
	}

	WvInput::Shutdown();
}

uint64_t WvApp::AdvanceTimer(const uint64_t uqwFrameIndex)
{
	switch (m_eMode)
	{
	case Mode::MODE_REPLAY:
	{
		const WvInputRecording::FrameRecord& sFrame =
			m_oRecording.GetFrame(uint32_t(uqwFrameIndex - 1));
		m_oTimer.Advance(sFrame.m_uqwStepMicroseconds);
		break;
	}

	case Mode::MODE_FIXED:
		m_oTimer.Advance(m_uqwFixedStepMicroseconds);
		break;

	case Mode::MODE_LIVE:
		m_oTimer.Update();
		break;
	}

	return m_oTimer.GetStepMicroseconds();
}

bool WvApp::CheckReplay(
	const uint64_t uqwFrameIndex, const uint64_t uqwStateHash
)
{
	const WvInputRecording::FrameRecord& sFrame =
		m_oRecording.GetFrame(uint32_t(uqwFrameIndex - 1));

	if (sFrame.m_uqwStateHash == uqwStateHash || m_uqwDivergedFrame != 0)
	{
		return true;
	}

	m_uqwDivergedFrame = uqwFrameIndex;
	std::printf(
		"replay diverged at frame %llu: recorded %016llx, got %016llx\n",
		(unsigned long long)uqwFrameIndex,
		(unsigned long long)sFrame.m_uqwStateHash,
		(unsigned long long)uqwStateHash
	);
	return false;
}

int WvApp::Loop()
{
	static const char* const s_apszModes[] = {"live", "fixed dt", "replay"};

	uint64_t uqwFrameIndex = 1;
	uint64_t uqwFramesRun = 0;

	while (true)
	{
		if (m_bWindow)
		{
			WvWindow::PumpMessages();

			if (WvWindow::IsQuitRequested())
			{
				break;
			}
		}

		if (m_eMode == Mode::MODE_REPLAY &&
			uqwFrameIndex > m_oRecording.GetFrameCount())
		{
			break;
		}

		const uint64_t uqwStep = AdvanceTimer(uqwFrameIndex);

		if (m_bAgent)
		{
			m_oAgent.Update(uqwFrameIndex);
		}

		WvInput::Poll();

		const uint32_t udwLiveCount =
			WvInput::Drain(uqwFrameIndex, m_asEvents, APP_MAX_EVENTS_PER_FRAME);

		const WvInputEvent* psEvents = m_asEvents;
		uint32_t udwEventCount = udwLiveCount;

		// Replay is exclusive: the recorded events drive the frame and
		// live input is drained and dropped, all but Escape.
		bool bQuitPressed = false;

		if (m_eMode == Mode::MODE_REPLAY)
		{
			for (uint32_t i = 0; i < udwLiveCount; i++)
			{
				const WvInputEvent& sEvent = m_asEvents[i];

				if (sEvent.m_eDevice == WvInputDevice::DEVICE_KEYBOARD &&
					sEvent.m_uwControl == uint16_t(WvKey::KEY_ESCAPE) &&
					sEvent.m_adwValues[0] != 0)
				{
					bQuitPressed = true;
				}
			}

			const WvInputRecording::FrameRecord& sFrame =
				m_oRecording.GetFrame(uint32_t(uqwFrameIndex - 1));
			psEvents = m_oRecording.GetEvents(sFrame);
			udwEventCount = sFrame.m_udwEventCount;
		}

		const WvInputFrame& oInput = m_oInputBuilder.Build(
			uqwFrameIndex, uqwStep, psEvents, udwEventCount
		);

		if (m_eMode != Mode::MODE_REPLAY &&
			oInput.GetPressCount(WvKey::KEY_ESCAPE) > 0)
		{
			bQuitPressed = true;
		}

		m_oGame.Update(m_oTimer, oInput);

		const uint64_t uqwStateHash = m_oGame.GetStateHash();

		if (m_bRecording && !m_oRecording.AddFrame(
								uqwStep, psEvents, udwEventCount, uqwStateHash
							))
		{
			std::fprintf(stderr, "recording full; stopped recording\n");
			m_bRecording = false;
		}

		if (m_eMode == Mode::MODE_REPLAY)
		{
			CheckReplay(uqwFrameIndex, uqwStateHash);
		}

		if (m_bWindow)
		{
			WvRenderStats sStats;
			sStats.m_pszMode = s_apszModes[uint32_t(m_eMode)];
			sStats.m_udwQueueFolded = WvInput::GetQueue().GetFoldedCount();
			sStats.m_udwQueueDropped = WvInput::GetQueue().GetDroppedCount();
			sStats.m_uqwStateHash = uqwStateHash;
			sStats.m_bAgent = m_bAgent;
			WvRenderer::Draw(m_oGame.GetWorld(), oInput, sStats);

			// Waits for the compositor's next vblank: one frame per
			// display refresh, without a GPU swapchain.
			DwmFlush();
		}

		uqwFramesRun++;

		if (bQuitPressed)
		{
			break;
		}

		if (m_uqwFrameLimit != 0 && uqwFrameIndex >= m_uqwFrameLimit)
		{
			break;
		}

		uqwFrameIndex++;
	}

	std::printf(
		"ran %llu frames, final state hash %016llx\n",
		(unsigned long long)uqwFramesRun,
		(unsigned long long)m_oGame.GetStateHash()
	);

	if (m_eMode == Mode::MODE_REPLAY)
	{
		if (m_uqwDivergedFrame == 0)
		{
			std::printf("replay matched on every frame\n");
			return 0;
		}

		return 2;
	}

	return 0;
}

} // namespace weev
