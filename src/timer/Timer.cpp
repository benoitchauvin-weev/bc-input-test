#include "timer/Timer.h"

#define WIN32_LEAN_AND_MEAN (1)
#define NOMINMAX (1)
#include <windows.h>

namespace weev
{

static uint64_t s_uqwTicksPerSecond = 0;

bool WvTimer::Init()
{
	LARGE_INTEGER sFrequency = {};

	if (QueryPerformanceFrequency(&sFrequency) == 0)
	{
		return false;
	}

	s_uqwTicksPerSecond = uint64_t(sFrequency.QuadPart);
	return s_uqwTicksPerSecond != 0;
}

uint64_t WvTimer::GetTicks()
{
	LARGE_INTEGER sCounter = {};
	QueryPerformanceCounter(&sCounter);
	return uint64_t(sCounter.QuadPart);
}

uint64_t WvTimer::GetTicksPerSecond()
{
	return s_uqwTicksPerSecond;
}

void WvTimer::Reset()
{
	m_uqwLastTicks = GetTicks();
	m_uqwStepMicroseconds = 0;
	m_uqwElapsedMicroseconds = 0;
	m_uqwFrameCount = 0;
}

void WvTimer::Update()
{
	const uint64_t uqwNow = GetTicks();
	const uint64_t uqwDelta = uqwNow - m_uqwLastTicks;
	m_uqwLastTicks = uqwNow;

	// Split before multiplying, so a large tick count cannot overflow.
	const uint64_t uqwSeconds = uqwDelta / s_uqwTicksPerSecond;
	const uint64_t uqwRemainder = uqwDelta % s_uqwTicksPerSecond;
	uint64_t uqwMicroseconds = uqwSeconds * 1000000 +
							   uqwRemainder * 1000000 / s_uqwTicksPerSecond;

	if (uqwMicroseconds > TIMER_MAX_STEP_MICROSECONDS)
	{
		uqwMicroseconds = TIMER_MAX_STEP_MICROSECONDS;
	}

	Advance(uqwMicroseconds);
}

void WvTimer::Advance(const uint64_t uqwStepMicroseconds)
{
	m_uqwStepMicroseconds = uqwStepMicroseconds;
	m_uqwElapsedMicroseconds += uqwStepMicroseconds;
	m_uqwFrameCount++;
}

uint64_t WvTimer::GetStepMicroseconds() const
{
	return m_uqwStepMicroseconds;
}

double WvTimer::GetTimeStep() const
{
	return double(m_uqwStepMicroseconds) / 1000000.0;
}

double WvTimer::GetTimeElapsed() const
{
	return double(m_uqwElapsedMicroseconds) / 1000000.0;
}

uint64_t WvTimer::GetFrameCount() const
{
	return m_uqwFrameCount;
}

} // namespace weev
