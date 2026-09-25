#pragma once

// Frame timing. The static half is the tick source; the instance half
// is the frame clock the game reads.
//
// A frame's step is either measured (live play), fixed (tests, agents)
// or given (replay, which feeds back the step each frame recorded). The
// step is carried in whole microseconds so a recording is exact.

#include <cstdint>

namespace weev
{

class WvTimer
{
public:
	static const uint64_t TIMER_MAX_STEP_MICROSECONDS = 250000;

	static bool Init();
	static uint64_t GetTicks();
	static uint64_t GetTicksPerSecond();

	void Reset();

	// Measured: the time since the last call, clamped so a debugger
	// break or a window drag reads as one long frame, not a leap.
	void Update();

	// Fixed or replayed: the step is given, the wall clock is not read.
	void Advance(uint64_t uqwStepMicroseconds);

	uint64_t GetStepMicroseconds() const;
	double GetTimeStep() const;
	double GetTimeElapsed() const;
	uint64_t GetFrameCount() const;

private:
	uint64_t m_uqwLastTicks = 0;
	uint64_t m_uqwStepMicroseconds = 0;
	uint64_t m_uqwElapsedMicroseconds = 0;
	uint64_t m_uqwFrameCount = 0;
};

} // namespace weev
