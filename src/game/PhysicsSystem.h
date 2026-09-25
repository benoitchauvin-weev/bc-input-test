#pragma once

// A mock platformer body per player: gravity, run acceleration, jumps
// with a cut when the button is let go early, a floor and two walls.
//
// There is no fixed game step, so physics chooses its own stepping: the
// frame's variable step is split into substeps no longer than
// PHYSICS_MAX_SUBSTEP.
//
// Reads: move intent, jump request, jump held.
// Writes: position, velocity, grounded, coyote time, jump cut, jump
// request (cleared), jump count.

#include "game/World.h"

namespace weev
{

class WvPhysicsSystem
{
public:
	static constexpr float PHYSICS_MAX_SUBSTEP = 1.0f / 240.0f;

	void Update(WvWorld& sWorld, float fStep);

private:
	static void StepPlayer(WvWorld& sWorld, uint32_t udwPlayer, float fStep);
};

} // namespace weev
