#pragma once

// The mock game's state, laid out ECS-style: one array per component,
// indexed by entity. Here the only entities are the players.
//
// Each system declares what it reads and writes, as the engine's job
// graph will need to schedule them.

#include "input/InputTypes.h"

namespace weev
{

constexpr uint32_t WORLD_PLAYER_COUNT = 2;

enum class WvAnimState : uint8_t
{
	ANIM_IDLE,
	ANIM_RUN,
	ANIM_JUMP,
	ANIM_FALL,
	ANIM_LAND,
};

struct WvWorld
{
	// Gameplay writes, physics reads.
	float m_afMoveIntent[WORLD_PLAYER_COUNT];
	float m_afJumpBuffer[WORLD_PLAYER_COUNT];
	uint8_t m_abyJumpHeld[WORLD_PLAYER_COUNT];
	uint8_t m_abyJumpRequest[WORLD_PLAYER_COUNT];

	// Physics writes; gameplay and animation read.
	float m_afPositionX[WORLD_PLAYER_COUNT];
	float m_afPositionY[WORLD_PLAYER_COUNT];
	float m_afVelocityX[WORLD_PLAYER_COUNT];
	float m_afVelocityY[WORLD_PLAYER_COUNT];
	float m_afCoyoteTime[WORLD_PLAYER_COUNT];
	uint8_t m_abyGrounded[WORLD_PLAYER_COUNT];
	uint8_t m_abyJumpCut[WORLD_PLAYER_COUNT];

	// Animation writes; the renderer reads.
	WvAnimState m_aeAnimState[WORLD_PLAYER_COUNT];
	float m_afAnimTime[WORLD_PLAYER_COUNT];
	float m_afRunPhase[WORLD_PLAYER_COUNT];
	float m_afScaleX[WORLD_PLAYER_COUNT];
	float m_afScaleY[WORLD_PLAYER_COUNT];

	uint32_t m_udwJumpCount[WORLD_PLAYER_COUNT];
};

} // namespace weev
