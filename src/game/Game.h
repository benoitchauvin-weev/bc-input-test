#pragma once

// The mock game: a world and the three systems that run over it, in the
// order their reads and writes require.
//
//   gameplay   reads input        writes intent
//   physics    reads intent       writes bodies
//   animation  reads bodies       writes poses

#include "game/AnimationSystem.h"
#include "game/GameplaySystem.h"
#include "game/PhysicsSystem.h"

namespace weev
{

class WvTimer;

class WvGame
{
public:
	void Init();

	// Once per rendered frame, with that frame's step and input. The
	// step comes from the timer, never from reading the clock here, so
	// a replay that feeds recorded steps reproduces the run.
	void Update(const WvTimer& oTimer, const WvInputFrame& oInput);

	// Over every component array, so a replay can name the first frame
	// whose outcome differs.
	uint64_t GetStateHash() const;

	const WvWorld& GetWorld() const;

private:
	WvWorld m_sWorld = {};
	WvGameplaySystem m_oGameplay;
	WvPhysicsSystem m_oPhysics;
	WvAnimationSystem m_oAnimation;
};

} // namespace weev
