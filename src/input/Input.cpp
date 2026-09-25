#include "input/Input.h"

#include "core/Assert.h"
#include "timer/Timer.h"

namespace weev
{

WvInputQueue WvInput::s_oQueue;
bool WvInput::s_bInitialised = false;
uint32_t WvInput::s_udwOwnerThread = 0;

bool WvInput::Init()
{
	s_oQueue.Reset();
	s_udwOwnerThread = PlatformGetThreadId();

	if (!PlatformInit())
	{
		return false;
	}

	s_bInitialised = true;
	return true;
}

void WvInput::Shutdown()
{
	if (!s_bInitialised)
	{
		return;
	}

	PlatformShutdown();
	s_oQueue.Reset();
	s_bInitialised = false;
}

bool WvInput::IsInitialised()
{
	return s_bInitialised;
}

void WvInput::AssertOwnerThread()
{
	WV_ASSERT(
		PlatformGetThreadId() == s_udwOwnerThread, "input is main-thread only"
	);
}

void WvInput::Push(const WvInputEvent& sEvent)
{
	PushAtFrame(sEvent, WvInputQueue::QUEUE_NEXT_FRAME);
}

void WvInput::PushAtFrame(
	const WvInputEvent& sEvent, const uint64_t uqwFrameIndex
)
{
	// Not an error: a window still sends messages while it is torn
	// down, after input has gone.
	if (!s_bInitialised)
	{
		return;
	}

	AssertOwnerThread();

	WvInputEvent sStamped = sEvent;
	sStamped.m_uqwTicks = WvTimer::GetTicks();
	s_oQueue.Push(sStamped, uqwFrameIndex);
}

void WvInput::Poll()
{
	if (!s_bInitialised)
	{
		return;
	}

	AssertOwnerThread();
	PlatformPoll();
}

uint32_t WvInput::Drain(
	const uint64_t uqwFrameIndex,
	WvInputEvent* const psOut,
	const uint32_t udwOutMax
)
{
	if (!s_bInitialised)
	{
		return 0;
	}

	AssertOwnerThread();
	return s_oQueue.Drain(uqwFrameIndex, psOut, udwOutMax);
}

const WvInputQueue& WvInput::GetQueue()
{
	return s_oQueue;
}

} // namespace weev
