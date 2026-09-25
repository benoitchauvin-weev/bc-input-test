#include "game/PhysicsSystem.h"

namespace weev
{

// World units are metres, +Y up, the floor at 0.
static const float PHYSICS_GRAVITY = -38.0f;
static const float PHYSICS_JUMP_SPEED = 13.0f;
static const float PHYSICS_JUMP_CUT = 0.45f;
static const float PHYSICS_RUN_SPEED = 7.5f;
static const float PHYSICS_GROUND_ACCELERATION = 70.0f;
static const float PHYSICS_AIR_ACCELERATION = 35.0f;
static const float PHYSICS_COYOTE_SECONDS = 0.08f;
static const float PHYSICS_WALL_X = 9.0f;

void WvPhysicsSystem::Update(WvWorld& sWorld, const float fStep)
{
	for (uint32_t i = 0; i < WORLD_PLAYER_COUNT; i++)
	{
		// The jump starts once, in the first substep.
		if (sWorld.m_abyJumpRequest[i] != 0)
		{
			sWorld.m_afVelocityY[i] = PHYSICS_JUMP_SPEED;
			sWorld.m_abyGrounded[i] = 0;
			sWorld.m_afCoyoteTime[i] = 0.0f;
			sWorld.m_abyJumpCut[i] = 0;
			sWorld.m_abyJumpRequest[i] = 0;
			sWorld.m_udwJumpCount[i]++;
		}

		const float fMaxSubstep = m_fMaxSubstep > 0.0f ? m_fMaxSubstep : fStep;
		float fRemaining = fStep;

		while (fRemaining > 0.0f)
		{
			const float fSubstep = fRemaining < fMaxSubstep ? fRemaining
															: fMaxSubstep;
			StepPlayer(sWorld, i, fSubstep);
			fRemaining -= fSubstep;
		}
	}
}

void WvPhysicsSystem::StepPlayer(
	WvWorld& sWorld, const uint32_t udwPlayer, const float fStep
)
{
	const uint32_t i = udwPlayer;
	const bool bGrounded = sWorld.m_abyGrounded[i] != 0;

	// Run toward the intended speed.
	const float fTarget = sWorld.m_afMoveIntent[i] * PHYSICS_RUN_SPEED;
	const float fAcceleration = bGrounded ? PHYSICS_GROUND_ACCELERATION
										  : PHYSICS_AIR_ACCELERATION;
	const float fDelta = fTarget - sWorld.m_afVelocityX[i];
	const float fMaxDelta = fAcceleration * fStep;

	if (fDelta > fMaxDelta)
	{
		sWorld.m_afVelocityX[i] += fMaxDelta;
	}
	else if (fDelta < -fMaxDelta)
	{
		sWorld.m_afVelocityX[i] -= fMaxDelta;
	}
	else
	{
		sWorld.m_afVelocityX[i] = fTarget;
	}

	// Variable jump height: letting go while rising cuts the jump, once.
	if (sWorld.m_abyJumpHeld[i] == 0 && sWorld.m_afVelocityY[i] > 0.0f &&
		sWorld.m_abyJumpCut[i] == 0)
	{
		sWorld.m_afVelocityY[i] *= PHYSICS_JUMP_CUT;
		sWorld.m_abyJumpCut[i] = 1;
	}

	if (!bGrounded)
	{
		sWorld.m_afVelocityY[i] += PHYSICS_GRAVITY * fStep;
		sWorld.m_afCoyoteTime[i] -= fStep;
	}

	// Semi-implicit Euler.
	sWorld.m_afPositionX[i] += sWorld.m_afVelocityX[i] * fStep;
	sWorld.m_afPositionY[i] += sWorld.m_afVelocityY[i] * fStep;

	if (sWorld.m_afPositionY[i] <= 0.0f)
	{
		sWorld.m_afPositionY[i] = 0.0f;

		if (sWorld.m_afVelocityY[i] < 0.0f)
		{
			sWorld.m_afVelocityY[i] = 0.0f;
		}

		sWorld.m_abyGrounded[i] = 1;
		sWorld.m_afCoyoteTime[i] = PHYSICS_COYOTE_SECONDS;
	}
	else
	{
		sWorld.m_abyGrounded[i] = 0;
	}

	if (sWorld.m_afPositionX[i] < -PHYSICS_WALL_X)
	{
		sWorld.m_afPositionX[i] = -PHYSICS_WALL_X;
		sWorld.m_afVelocityX[i] = 0.0f;
	}
	else if (sWorld.m_afPositionX[i] > PHYSICS_WALL_X)
	{
		sWorld.m_afPositionX[i] = PHYSICS_WALL_X;
		sWorld.m_afVelocityX[i] = 0.0f;
	}
}

void WvPhysicsSystem::SetMaxSubstep(const float fMaxSubstep)
{
	m_fMaxSubstep = fMaxSubstep;
}

} // namespace weev
