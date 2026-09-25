#pragma once

// A mock animation state machine: idle, run, jump, fall, land, driving
// a squash-and-stretch scale the renderer draws.
//
// Reads: velocity, grounded.
// Writes: animation state and time, run phase, scale.

#include "game/World.h"

namespace weev
{

class WvAnimationSystem
{
public:
	void Update(WvWorld& sWorld, float fStep);

private:
	static WvAnimState ChooseState(const WvWorld& sWorld, uint32_t udwPlayer);
};

} // namespace weev
