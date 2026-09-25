#include "test/FrameRateProbe.h"

#include "game/Game.h"
#include "input/InputFrameBuilder.h"
#include "timer/Timer.h"

#include <cmath>
#include <cstdio>

namespace weev
{

static const uint32_t PROBE_JUMP_COUNT = 2;
static const uint64_t PROBE_DURATION_MICROSECONDS = 6000000;
static const uint32_t PROBE_MAX_EVENTS = 8;

struct WvProbeEvent
{
	uint64_t m_uqwTimeMicroseconds;
	WvKey m_eKey;
	bool m_bDown;
};

// Run right with a short hop (jump held 0.25 s, so the cut applies),
// stop, then run left with a full jump (held 1 s).
static const WvProbeEvent s_asProbeScript[] = {
	{200000, WvKey::KEY_D, true},
	{1000000, WvKey::KEY_SPACE, true},
	{1250000, WvKey::KEY_SPACE, false},
	{1800000, WvKey::KEY_D, false},
	{2400000, WvKey::KEY_A, true},
	{3000000, WvKey::KEY_SPACE, true},
	{4000000, WvKey::KEY_SPACE, false},
};

struct WvProbeConfig
{
	const char* m_pszName;
	uint64_t m_uqwStepMicroseconds; // 0 for jitter
};

struct WvProbeResult
{
	float m_afApex[PROBE_JUMP_COUNT];
	float m_afAirSeconds[PROBE_JUMP_COUNT];
	float m_afTakeoffX[PROBE_JUMP_COUNT];
	float m_afLandX[PROBE_JUMP_COUNT];
	uint32_t m_udwJumps;
	uint32_t m_udwFrames;
};

class WvFrameRateProbeRun
{
public:
	static WvProbeResult Run(const WvProbeConfig& sConfig, float fMaxSubstep);

private:
	static uint64_t NextStep(const WvProbeConfig& sConfig, uint32_t& udwSeed);
};

uint64_t WvFrameRateProbeRun::NextStep(
	const WvProbeConfig& sConfig, uint32_t& udwSeed
)
{
	if (sConfig.m_uqwStepMicroseconds != 0)
	{
		return sConfig.m_uqwStepMicroseconds;
	}

	// Jitter: 6 to 20 ms, from a fixed seed so the run repeats.
	udwSeed = udwSeed * 1664525u + 1013904223u;
	return 6000 + (udwSeed >> 8) % 14000;
}

WvProbeResult WvFrameRateProbeRun::Run(
	const WvProbeConfig& sConfig, const float fMaxSubstep
)
{
	static WvGame s_oGame;
	static WvInputFrameBuilder s_oBuilder;

	s_oGame.Init();
	s_oGame.SetPhysicsMaxSubstep(fMaxSubstep);
	s_oBuilder.Reset();

	WvTimer oTimer;
	oTimer.Reset();

	WvProbeResult sResult = {};
	WvInputEvent asEvents[PROBE_MAX_EVENTS] = {};
	const uint32_t udwScriptCount = sizeof(s_asProbeScript) /
									sizeof(s_asProbeScript[0]);

	uint32_t udwSeed = 1;
	uint32_t udwNextEvent = 0;
	uint64_t uqwTime = 0;
	uint64_t uqwFrame = 1;
	uint64_t uqwTakeoffTime = 0;
	bool bAirborne = false;

	while (uqwTime < PROBE_DURATION_MICROSECONDS)
	{
		const uint64_t uqwStep = NextStep(sConfig, udwSeed);
		uqwTime += uqwStep;
		oTimer.Advance(uqwStep);

		uint32_t udwCount = 0;

		while (udwNextEvent < udwScriptCount &&
			   s_asProbeScript[udwNextEvent].m_uqwTimeMicroseconds <= uqwTime &&
			   udwCount < PROBE_MAX_EVENTS)
		{
			const WvProbeEvent& sEvent = s_asProbeScript[udwNextEvent];
			asEvents[udwCount] = WvInputEvent::MakeKey(
				sEvent.m_eKey, sEvent.m_bDown, WvInputSource::SOURCE_AGENT, 0
			);
			udwCount++;
			udwNextEvent++;
		}

		const WvInputFrame& oInput =
			s_oBuilder.Build(uqwFrame, uqwStep, asEvents, udwCount);
		s_oGame.Update(oTimer, oInput);

		const WvWorld& sWorld = s_oGame.GetWorld();
		const uint32_t udwJump = sResult.m_udwJumps;
		const bool bGrounded = sWorld.m_abyGrounded[0] != 0;

		if (!bAirborne && !bGrounded && udwJump < PROBE_JUMP_COUNT)
		{
			bAirborne = true;
			uqwTakeoffTime = uqwTime;
			sResult.m_afTakeoffX[udwJump] = sWorld.m_afPositionX[0];
		}

		if (bAirborne)
		{
			if (sWorld.m_afPositionY[0] > sResult.m_afApex[udwJump])
			{
				sResult.m_afApex[udwJump] = sWorld.m_afPositionY[0];
			}

			if (bGrounded)
			{
				bAirborne = false;
				sResult.m_afAirSeconds[udwJump] =
					float(uqwTime - uqwTakeoffTime) / 1000000.0f;
				sResult.m_afLandX[udwJump] = sWorld.m_afPositionX[0];
				sResult.m_udwJumps++;
			}
		}

		uqwFrame++;
	}

	sResult.m_udwFrames = uint32_t(uqwFrame - 1);
	return sResult;
}

void WvFrameRateProbe::Run()
{
	static const WvProbeConfig s_asConfigs[] = {
		{"30 Hz", 33333},
		{"60 Hz", 16667},
		{"120 Hz", 8333},
		{"144 Hz", 6944},
		{"240 Hz", 4167},
		{"jitter 6-20 ms", 0},
	};

	struct PhysicsMode
	{
		const char* m_pszName;
		float m_fMaxSubstep;
	};

	static const PhysicsMode s_asModes[] = {
		{"physics substeps at 1/240 s or less (the sandbox default)",
		 WvPhysicsSystem::PHYSICS_MAX_SUBSTEP},
		{"physics takes one step per frame (naive variable step)", 0.0f},
	};

	const uint32_t udwConfigCount = sizeof(s_asConfigs) /
									sizeof(s_asConfigs[0]);
	const uint32_t udwModeCount = sizeof(s_asModes) / sizeof(s_asModes[0]);

	for (uint32_t m = 0; m < udwModeCount; m++)
	{
		WvProbeResult asResults[sizeof(s_asConfigs) / sizeof(s_asConfigs[0])] =
			{};

		for (uint32_t i = 0; i < udwConfigCount; i++)
		{
			asResults[i] = WvFrameRateProbeRun::Run(
				s_asConfigs[i], s_asModes[m].m_fMaxSubstep
			);
		}

		// 60 Hz is the reference every other rate is compared with.
		const WvProbeResult& sReference = asResults[1];

		std::printf("\n%s\n", s_asModes[m].m_pszName);
		std::printf(
			"%-15s %6s | %-37s | %-37s\n",
			"",
			"",
			"short hop (jump held 0.25 s)",
			"full jump (jump held 1 s)"
		);
		std::printf(
			"%-15s %6s | %7s %7s %7s %6s %5s | %7s %7s %7s %6s %5s\n",
			"rate",
			"frames",
			"apex m",
			"vs 60",
			"dist m",
			"vs 60",
			"air s",
			"apex m",
			"vs 60",
			"dist m",
			"vs 60",
			"air s"
		);

		for (uint32_t i = 0; i < udwConfigCount; i++)
		{
			const WvProbeResult& s = asResults[i];

			std::printf("%-15s %6u |", s_asConfigs[i].m_pszName, s.m_udwFrames);

			for (uint32_t j = 0; j < PROBE_JUMP_COUNT; j++)
			{
				const float fDistance =
					std::fabs(s.m_afLandX[j] - s.m_afTakeoffX[j]);
				const float fReferenceDistance = std::fabs(
					sReference.m_afLandX[j] - sReference.m_afTakeoffX[j]
				);
				const float fApexDelta =
					(s.m_afApex[j] - sReference.m_afApex[j]) /
					sReference.m_afApex[j] * 100.0f;
				const float fDistanceDelta = (fDistance - fReferenceDistance) /
											 fReferenceDistance * 100.0f;

				std::printf(
					" %7.3f %+6.1f%% %7.3f %+5.1f%% %5.3f |",
					double(s.m_afApex[j]),
					double(fApexDelta),
					double(fDistance),
					double(fDistanceDelta),
					double(s.m_afAirSeconds[j])
				);
			}

			std::printf("\n");
		}
	}

	std::printf(
		"\nair time is measured at frame ends, so it is quantised to the "
		"frame length.\n"
	);
}

} // namespace weev
