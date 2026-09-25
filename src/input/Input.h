#pragma once

// The static platform side of input: the one queue, and the backend
// that feeds it. Everything that produces input calls Push, a human's
// keyboard as much as a test or an agent.
//
// Main thread only, and asserted: Windows and the web both deliver input
// there. A platform that delivers on its own thread (Android) will need
// a queue per producer.
//
// Frame indices start at 1. PushAtFrame(e, 0) means the next frame.

#include "input/InputQueue.h"

namespace weev
{

class WvInput
{
public:
	// Needs no window: the backend attaches to one later, so a test can
	// initialise input with nothing on screen.
	static bool Init();
	static void Shutdown();
	static bool IsInitialised();

	static void Push(const WvInputEvent& sEvent);
	static void PushAtFrame(const WvInputEvent& sEvent, uint64_t uqwFrameIndex);

	// Gamepads are polled, not delivered. Call as late as possible
	// before the frame is built, for latency.
	static void Poll();

	static uint32_t Drain(
		uint64_t uqwFrameIndex, WvInputEvent* psOut, uint32_t udwOutMax
	);

	static const WvInputQueue& GetQueue();

private:
	static bool PlatformInit();
	static void PlatformShutdown();
	static void PlatformPoll();
	static uint32_t PlatformGetThreadId();

	static void AssertOwnerThread();

	static WvInputQueue s_oQueue;
	static bool s_bInitialised;
	static uint32_t s_udwOwnerThread;
};

} // namespace weev
