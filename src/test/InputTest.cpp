#include "test/InputTest.h"

#include "input/InputFrameBuilder.h"
#include "input/InputKeyMap.h"
#include "input/InputQueue.h"
#include "input/InputRecording.h"

#include <cstdio>
#include <cstring>

#define WIN32_LEAN_AND_MEAN (1)
#define NOMINMAX (1)
#include <windows.h>

#define WV_TEST_CHECK(Condition)                                        \
	do                                                                  \
	{                                                                   \
		if (!(Condition))                                               \
		{                                                               \
			std::printf(                                                \
				"  FAILED %s:%d (%s)\n", __FILE__, __LINE__, #Condition \
			);                                                          \
			return false;                                               \
		}                                                               \
	} while (false)

namespace weev
{

// The hash of the frame TestGoldenHash builds. A constant, so every
// platform that runs the test checks the same value. It changes only
// when the frame layout or the builder's rules change, on purpose.
static const uint64_t INPUT_TEST_GOLDEN_HASH = 0x5e67b17b79e6eebbull;

static const WvInputSource SOURCE_HUMAN = WvInputSource::SOURCE_HUMAN;
static const WvInputSource SOURCE_AGENT = WvInputSource::SOURCE_AGENT;

// Big objects, kept off the stack.
static WvInputFrameBuilder s_oBuilder;
static WvInputQueue s_oQueue;
static WvInputEvent s_asDrained[1024];

const WvInputTest::TestCase WvInputTest::s_asTests[] = {
	{"tap inside one frame", &TestTapInsideFrame},
	{"held across frames", &TestHeldAcrossFrames},
	{"two keyboards hold one key", &TestTwoKeyboards},
	{"focus loss keeps agent keys", &TestFocusLostKeepsAgent},
	{"full queue folds presses", &TestQueueFullFolds},
	{"motion merged, not queued", &TestMotionMerged},
	{"push aimed at a frame", &TestTargetedPush},
	{"gamepad slots and generations", &TestGamepadSlots},
	{"double buffer", &TestDoubleBuffer},
	{"key maps", &TestKeyMaps},
	{"axis to float", &TestAxisFloat},
	{"recording round trip", &TestRecordingRoundTrip},
	{"golden hash", &TestGoldenHash},
};

bool WvInputTest::Run()
{
	const uint32_t udwCount = sizeof(s_asTests) / sizeof(s_asTests[0]);
	uint32_t udwPassed = 0;

	for (uint32_t i = 0; i < udwCount; i++)
	{
		s_oBuilder.Reset();
		s_oQueue.Reset();

		const bool bPassed = s_asTests[i].m_pfnRun();
		std::printf(
			"%s  %s\n", bPassed ? "pass" : "FAIL", s_asTests[i].m_pszName
		);

		if (bPassed)
		{
			udwPassed++;
		}
	}

	std::printf("%u of %u input tests passed\n", udwPassed, udwCount);
	return udwPassed == udwCount;
}

bool WvInputTest::TestTapInsideFrame()
{
	const WvInputEvent asEvents[] = {
		WvInputEvent::MakeKey(WvKey::KEY_SPACE, true, SOURCE_HUMAN, 0),
		WvInputEvent::MakeKey(WvKey::KEY_SPACE, false, SOURCE_HUMAN, 0),
	};

	const WvInputFrame& oFirst = s_oBuilder.Build(1, 16667, asEvents, 2);
	WV_TEST_CHECK(oFirst.GetPressCount(WvKey::KEY_SPACE) == 1);
	WV_TEST_CHECK(oFirst.GetReleaseCount(WvKey::KEY_SPACE) == 1);
	WV_TEST_CHECK(!oFirst.IsDown(WvKey::KEY_SPACE));

	const WvInputFrame& oSecond = s_oBuilder.Build(2, 16667, nullptr, 0);
	WV_TEST_CHECK(oSecond.GetPressCount(WvKey::KEY_SPACE) == 0);
	WV_TEST_CHECK(oSecond.GetReleaseCount(WvKey::KEY_SPACE) == 0);
	return true;
}

bool WvInputTest::TestHeldAcrossFrames()
{
	const WvInputEvent sDown =
		WvInputEvent::MakeKey(WvKey::KEY_D, true, SOURCE_HUMAN, 0);

	s_oBuilder.Build(1, 16667, &sDown, 1);
	const WvInputFrame& oHeld = s_oBuilder.Build(2, 16667, nullptr, 0);
	WV_TEST_CHECK(oHeld.IsDown(WvKey::KEY_D));
	WV_TEST_CHECK(oHeld.GetPressCount(WvKey::KEY_D) == 0);
	WV_TEST_CHECK(oHeld.GetFrameIndex() == 2);
	return true;
}

bool WvInputTest::TestTwoKeyboards()
{
	const WvInputEvent asDown[] = {
		WvInputEvent::MakeKey(WvKey::KEY_LEFT, true, SOURCE_HUMAN, 0),
		WvInputEvent::MakeKey(WvKey::KEY_LEFT, true, SOURCE_HUMAN, 1),
	};

	const WvInputFrame& oBoth = s_oBuilder.Build(1, 16667, asDown, 2);
	WV_TEST_CHECK(oBoth.GetPressCount(WvKey::KEY_LEFT) == 1);

	// Keyboard 1 lets go: keyboard 0 still holds it.
	const WvInputEvent sUpOne =
		WvInputEvent::MakeKey(WvKey::KEY_LEFT, false, SOURCE_HUMAN, 1);
	const WvInputFrame& oOne = s_oBuilder.Build(2, 16667, &sUpOne, 1);
	WV_TEST_CHECK(oOne.IsDown(WvKey::KEY_LEFT));
	WV_TEST_CHECK(oOne.GetReleaseCount(WvKey::KEY_LEFT) == 0);

	const WvInputEvent sUpZero =
		WvInputEvent::MakeKey(WvKey::KEY_LEFT, false, SOURCE_HUMAN, 0);
	const WvInputFrame& oNone = s_oBuilder.Build(3, 16667, &sUpZero, 1);
	WV_TEST_CHECK(!oNone.IsDown(WvKey::KEY_LEFT));
	WV_TEST_CHECK(oNone.GetReleaseCount(WvKey::KEY_LEFT) == 1);
	return true;
}

bool WvInputTest::TestFocusLostKeepsAgent()
{
	const WvInputEvent asDown[] = {
		WvInputEvent::MakeKey(WvKey::KEY_A, true, SOURCE_HUMAN, 0),
		WvInputEvent::MakeKey(WvKey::KEY_RIGHT, true, SOURCE_AGENT, 0),
		WvInputEvent::MakePadButton(
			0, WvGamepadButton::BUTTON_SOUTH, true, SOURCE_HUMAN
		),
	};
	s_oBuilder.Build(1, 16667, asDown, 3);

	const WvInputEvent sFocusLost = WvInputEvent::MakeFocusLost(SOURCE_HUMAN);
	const WvInputFrame& oFrame = s_oBuilder.Build(2, 16667, &sFocusLost, 1);

	WV_TEST_CHECK(oFrame.IsFocusLostThisFrame());
	WV_TEST_CHECK(!oFrame.IsDown(WvKey::KEY_A));
	WV_TEST_CHECK(oFrame.GetReleaseCount(WvKey::KEY_A) == 1);
	WV_TEST_CHECK(oFrame.IsDown(WvKey::KEY_RIGHT));
	WV_TEST_CHECK(!oFrame.IsGamepadDown(0, WvGamepadButton::BUTTON_SOUTH));
	return true;
}

bool WvInputTest::TestQueueFullFolds()
{
	// 600 alternating edges on one key: far more than the queue holds.
	const uint32_t udwEdges = 600;

	for (uint32_t i = 0; i < udwEdges; i++)
	{
		s_oQueue.Push(
			WvInputEvent::MakeKey(WvKey::KEY_Z, (i % 2) == 0, SOURCE_HUMAN, 0),
			WvInputQueue::QUEUE_NEXT_FRAME
		);
	}

	s_oQueue.Push(
		WvInputEvent::MakeKey(WvKey::KEY_X, true, SOURCE_HUMAN, 0),
		WvInputQueue::QUEUE_NEXT_FRAME
	);

	// Non-button events still get a queue slot while edges fold... but
	// the queue is full, so this one is dropped and counted.
	s_oQueue.Push(
		WvInputEvent::MakeActivity(WvInputActivity::ACTIVITY_OVERLAY),
		WvInputQueue::QUEUE_NEXT_FRAME
	);

	WV_TEST_CHECK(s_oQueue.GetFoldedCount() > 0);
	WV_TEST_CHECK(s_oQueue.GetDroppedCount() == 1);

	const uint32_t udwCount = s_oQueue.Drain(1, s_asDrained, 1024);
	WV_TEST_CHECK(udwCount == udwEdges + 1);

	const WvInputFrame& oFrame =
		s_oBuilder.Build(1, 16667, s_asDrained, udwCount);

	// Counts saturate at 255 in the snapshot; the edges were all there.
	WV_TEST_CHECK(oFrame.GetPressCount(WvKey::KEY_Z) == 255);
	WV_TEST_CHECK(oFrame.GetReleaseCount(WvKey::KEY_Z) == 255);
	WV_TEST_CHECK(!oFrame.IsDown(WvKey::KEY_Z));
	WV_TEST_CHECK(oFrame.IsDown(WvKey::KEY_X));
	WV_TEST_CHECK(s_oQueue.GetPendingCount() == 0);
	return true;
}

bool WvInputTest::TestMotionMerged()
{
	for (uint32_t i = 0; i < 1000; i++)
	{
		s_oQueue.Push(
			WvInputEvent::MakeMouseMotion(
				WvMouseMotion::MOTION_MOVE, 1, -2, SOURCE_HUMAN
			),
			WvInputQueue::QUEUE_NEXT_FRAME
		);
	}

	s_oQueue.Push(
		WvInputEvent::MakeMousePosition(100, 200, SOURCE_HUMAN),
		WvInputQueue::QUEUE_NEXT_FRAME
	);
	s_oQueue.Push(
		WvInputEvent::MakeMousePosition(300, 400, SOURCE_HUMAN),
		WvInputQueue::QUEUE_NEXT_FRAME
	);

	WV_TEST_CHECK(s_oQueue.GetPendingCount() == 2);

	const uint32_t udwCount = s_oQueue.Drain(1, s_asDrained, 1024);
	const WvInputFrame& oFrame =
		s_oBuilder.Build(1, 16667, s_asDrained, udwCount);

	int32_t dwX = 0;
	int32_t dwY = 0;
	oFrame.GetMouseDelta(dwX, dwY);
	WV_TEST_CHECK(dwX == 1000 && dwY == -2000);
	oFrame.GetMousePosition(dwX, dwY);
	WV_TEST_CHECK(dwX == 300 && dwY == 400);

	// Delta belongs to one frame; position persists.
	const WvInputFrame& oNext = s_oBuilder.Build(2, 16667, nullptr, 0);
	oNext.GetMouseDelta(dwX, dwY);
	WV_TEST_CHECK(dwX == 0 && dwY == 0);
	oNext.GetMousePosition(dwX, dwY);
	WV_TEST_CHECK(dwX == 300 && dwY == 400);
	return true;
}

bool WvInputTest::TestTargetedPush()
{
	s_oQueue.Push(
		WvInputEvent::MakeKey(WvKey::KEY_UP, true, SOURCE_AGENT, 0), 5
	);
	s_oQueue.Push(
		WvInputEvent::MakeKey(WvKey::KEY_W, true, SOURCE_HUMAN, 0),
		WvInputQueue::QUEUE_NEXT_FRAME
	);

	WV_TEST_CHECK(s_oQueue.Drain(3, s_asDrained, 1024) == 1);
	WV_TEST_CHECK(s_asDrained[0].m_uwControl == uint16_t(WvKey::KEY_W));
	WV_TEST_CHECK(s_oQueue.Drain(4, s_asDrained, 1024) == 0);
	WV_TEST_CHECK(s_oQueue.Drain(5, s_asDrained, 1024) == 1);
	WV_TEST_CHECK(s_asDrained[0].m_uwControl == uint16_t(WvKey::KEY_UP));
	return true;
}

bool WvInputTest::TestGamepadSlots()
{
	const WvInputEvent asConnect[] = {
		WvInputEvent::MakeConnect(2, 0x1234, WvGamepadFamily::FAMILY_XBOX, 0),
		WvInputEvent::MakePadButton(
			2, WvGamepadButton::BUTTON_SOUTH, true, SOURCE_HUMAN
		),
		WvInputEvent::MakePadAxis(
			2, WvGamepadAxis::AXIS_LEFT_X, 20000, SOURCE_HUMAN
		),
	};

	const WvInputFrame& oConnected = s_oBuilder.Build(1, 16667, asConnect, 3);
	WV_TEST_CHECK(oConnected.IsGamepadConnected(2));
	WV_TEST_CHECK(oConnected.GetGamepad(2).m_udwGeneration == 1);
	WV_TEST_CHECK(
		oConnected.GetGamepadPressCount(2, WvGamepadButton::BUTTON_SOUTH) == 1
	);

	const WvInputEvent sDisconnect = WvInputEvent::MakeDisconnect(2);
	const WvInputFrame& oGone = s_oBuilder.Build(2, 16667, &sDisconnect, 1);
	WV_TEST_CHECK(!oGone.IsGamepadConnected(2));
	WV_TEST_CHECK(
		oGone.GetGamepadReleaseCount(2, WvGamepadButton::BUTTON_SOUTH) == 1
	);
	WV_TEST_CHECK(oGone.GetGamepadAxis(2, WvGamepadAxis::AXIS_LEFT_X) == 0);

	// The slot keeps its id while empty, and the same device returning
	// is a new generation of the same slot.
	WV_TEST_CHECK(oGone.GetGamepad(2).m_udwStableId == 0x1234);

	const WvInputFrame& oBack = s_oBuilder.Build(3, 16667, asConnect, 1);
	WV_TEST_CHECK(oBack.IsGamepadConnected(2));
	WV_TEST_CHECK(oBack.GetGamepad(2).m_udwGeneration == 2);
	return true;
}

bool WvInputTest::TestDoubleBuffer()
{
	const WvInputFrame& oFirst = s_oBuilder.Build(1, 16667, nullptr, 0);
	const WvInputFrame& oSecond = s_oBuilder.Build(2, 16667, nullptr, 0);

	// Frame 1 is still intact while frame 2 is the front...
	WV_TEST_CHECK(&oFirst != &oSecond);
	WV_TEST_CHECK(oFirst.GetFrameIndex() == 1);
	WV_TEST_CHECK(&s_oBuilder.GetFront() == &oSecond);

	// ...and its buffer is the one frame 3 reuses.
	const WvInputFrame& oThird = s_oBuilder.Build(3, 16667, nullptr, 0);
	WV_TEST_CHECK(&oThird == &oFirst);
	WV_TEST_CHECK(oSecond.GetFrameIndex() == 2);
	return true;
}

bool WvInputTest::TestKeyMaps()
{
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x1E, false, 'A') == WvKey::KEY_A
	);
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x10, false, 'A') == WvKey::KEY_Q
	);
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x45, false, VK_PAUSE) ==
		WvKey::KEY_PAUSE
	);
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x45, true, VK_NUMLOCK) ==
		WvKey::KEY_NUM_LOCK
	);
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x48, true, VK_UP) == WvKey::KEY_UP
	);
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x48, false, VK_NUMPAD8) ==
		WvKey::KEY_KP_8
	);
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x1D, true, VK_CONTROL) ==
		WvKey::KEY_RIGHT_CTRL
	);
	WV_TEST_CHECK(
		WvInputKeyMap::FromWinScancode(0x54, false, VK_SNAPSHOT) ==
		WvKey::KEY_PRINT_SCREEN
	);
	WV_TEST_CHECK(WvInputKeyMap::FromDomCode("KeyA") == WvKey::KEY_A);
	WV_TEST_CHECK(WvInputKeyMap::FromDomCode("ArrowUp") == WvKey::KEY_UP);
	WV_TEST_CHECK(WvInputKeyMap::FromDomCode("") == WvKey::KEY_NONE);
	WV_TEST_CHECK(
		WvInputKeyMap::FromDomCode("Unidentified") == WvKey::KEY_NONE
	);
	return true;
}

bool WvInputTest::TestAxisFloat()
{
	const WvInputEvent asAxes[] = {
		WvInputEvent::MakePadAxis(
			0, WvGamepadAxis::AXIS_LEFT_X, -32768, SOURCE_HUMAN
		),
		WvInputEvent::MakePadAxis(
			0, WvGamepadAxis::AXIS_LEFT_Y, 32767, SOURCE_HUMAN
		),
	};

	const WvInputFrame& oFrame = s_oBuilder.Build(1, 16667, asAxes, 2);
	WV_TEST_CHECK(
		oFrame.GetGamepadAxisFloat(0, WvGamepadAxis::AXIS_LEFT_X) == -1.0f
	);
	WV_TEST_CHECK(
		oFrame.GetGamepadAxisFloat(0, WvGamepadAxis::AXIS_LEFT_Y) == 1.0f
	);
	WV_TEST_CHECK(
		oFrame.GetGamepadAxisFloat(0, WvGamepadAxis::AXIS_RIGHT_X) == 0.0f
	);
	return true;
}

bool WvInputTest::TestRecordingRoundTrip()
{
	static WvInputRecording s_oRecording;
	static WvInputRecording s_oLoaded;
	s_oRecording.Reset();

	WvInputEvent sDown =
		WvInputEvent::MakeKey(WvKey::KEY_J, true, SOURCE_AGENT, 0);
	sDown.m_uqwTicks = 123456;

	WV_TEST_CHECK(s_oRecording.AddFrame(16667, &sDown, 1, 0xABCD));
	WV_TEST_CHECK(s_oRecording.AddFrame(8000, nullptr, 0, 0xBEEF));

	char szDirectory[MAX_PATH] = {};
	char szPath[MAX_PATH] = {};
	GetTempPathA(MAX_PATH, szDirectory);
	std::snprintf(szPath, sizeof(szPath), "%sbc_input_test.wvir", szDirectory);

	WV_TEST_CHECK(s_oRecording.Save(szPath));
	WV_TEST_CHECK(s_oLoaded.Load(szPath));
	DeleteFileA(szPath);

	WV_TEST_CHECK(s_oLoaded.GetFrameCount() == 2);

	const WvInputRecording::FrameRecord& sFirst = s_oLoaded.GetFrame(0);
	WV_TEST_CHECK(sFirst.m_uqwStepMicroseconds == 16667);
	WV_TEST_CHECK(sFirst.m_uqwStateHash == 0xABCD);
	WV_TEST_CHECK(sFirst.m_udwEventCount == 1);

	const WvInputEvent& sLoaded = s_oLoaded.GetEvents(sFirst)[0];
	WV_TEST_CHECK(sLoaded.m_uwControl == uint16_t(WvKey::KEY_J));
	WV_TEST_CHECK(sLoaded.m_eSource == SOURCE_AGENT);

	// Timestamps never reach a recording.
	WV_TEST_CHECK(sLoaded.m_uqwTicks == 0);

	WV_TEST_CHECK(s_oLoaded.GetFrame(1).m_uqwStepMicroseconds == 8000);
	return true;
}

bool WvInputTest::TestGoldenHash()
{
	const WvInputEvent asFirst[] = {
		WvInputEvent::MakeViewport(1280, 720, 100),
		WvInputEvent::MakeActivity(WvInputActivity::ACTIVITY_FOCUSED),
		WvInputEvent::MakeConnect(
			0, 0x58490000, WvGamepadFamily::FAMILY_XBOX, 0
		),
		WvInputEvent::MakeKey(WvKey::KEY_D, true, SOURCE_HUMAN, 0),
		WvInputEvent::MakeKey(WvKey::KEY_SPACE, true, SOURCE_HUMAN, 0),
		WvInputEvent::MakeKey(WvKey::KEY_SPACE, false, SOURCE_HUMAN, 0),
		WvInputEvent::MakeMousePosition(640 * 16, 360 * 16, SOURCE_HUMAN),
	};
	const WvInputEvent asSecond[] = {
		WvInputEvent::MakePadButton(
			0, WvGamepadButton::BUTTON_SOUTH, true, SOURCE_HUMAN
		),
		WvInputEvent::MakePadAxis(
			0, WvGamepadAxis::AXIS_LEFT_X, -12000, SOURCE_HUMAN
		),
		WvInputEvent::MakeMouseMotion(
			WvMouseMotion::MOTION_MOVE, 7, -3, SOURCE_HUMAN
		),
		WvInputEvent::MakeKey(WvKey::KEY_RIGHT, true, SOURCE_AGENT, 0),
	};

	s_oBuilder.Build(1, 16667, asFirst, sizeof(asFirst) / sizeof(asFirst[0]));
	const WvInputFrame& oFrame = s_oBuilder.Build(
		2, 16667, asSecond, sizeof(asSecond) / sizeof(asSecond[0])
	);

	const uint64_t uqwHash = oFrame.Hash();

	if (uqwHash != INPUT_TEST_GOLDEN_HASH)
	{
		std::printf(
			"  golden hash is %016llx, expected %016llx\n",
			(unsigned long long)uqwHash,
			(unsigned long long)INPUT_TEST_GOLDEN_HASH
		);
	}

	WV_TEST_CHECK(uqwHash == INPUT_TEST_GOLDEN_HASH);
	return true;
}

} // namespace weev
