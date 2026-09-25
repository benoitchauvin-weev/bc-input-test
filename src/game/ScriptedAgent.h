#pragma once

// A stand-in for an AI agent playing player 2. It never touches the
// game: it pushes ordinary key events, tagged SOURCE_AGENT, aimed at
// exact frames with PushAtFrame. Same path as a human's keyboard.
//
// Its keys survive the window losing focus, because focus loss releases
// only what a human holds.

#include <cstdint>

namespace weev
{

class WvScriptedAgent
{
public:
	static const uint64_t AGENT_CYCLE_FRAMES = 360;

	// Schedules the next cycle of the script when a cycle begins.
	void Update(uint64_t uqwFrameIndex);

private:
	static void ScheduleCycle(uint64_t uqwStartFrame);

	uint64_t m_uqwNextCycleFrame = 1;
};

} // namespace weev
