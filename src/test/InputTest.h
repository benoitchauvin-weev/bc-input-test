#pragma once

// --test: the input system's self-tests. They drive a queue and a frame
// builder directly with synthetic events, so no window, backend or
// global state is involved, as the engine's --test runs before any
// window exists.

namespace weev
{

class WvInputTest
{
public:
	static bool Run();

private:
	typedef bool (*TestFunction)();

	struct TestCase
	{
		const char* m_pszName;
		TestFunction m_pfnRun;
	};

	static bool TestTapInsideFrame();
	static bool TestHeldAcrossFrames();
	static bool TestTwoKeyboards();
	static bool TestFocusLostKeepsAgent();
	static bool TestQueueFullFolds();
	static bool TestMotionMerged();
	static bool TestTargetedPush();
	static bool TestGamepadSlots();
	static bool TestDoubleBuffer();
	static bool TestKeyMaps();
	static bool TestAxisFloat();
	static bool TestRecordingRoundTrip();
	static bool TestGoldenHash();

	static const TestCase s_asTests[];
};

} // namespace weev
