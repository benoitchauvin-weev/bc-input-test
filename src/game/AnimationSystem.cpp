#include "game/AnimationSystem.h"

#include <cmath>

namespace weev
{

static const float ANIMATION_LAND_SECONDS = 0.12f;
static const float ANIMATION_RUN_THRESHOLD = 0.2f;
static const float ANIMATION_BLEND_RATE = 18.0f;

WvAnimState WvAnimationSystem::ChooseState(
	const WvWorld& sWorld, const uint32_t udwPlayer
)
{
	const uint32_t i = udwPlayer;
	const WvAnimState ePrevious = sWorld.m_aeAnimState[i];

	if (sWorld.m_abyGrounded[i] == 0)
	{
		return sWorld.m_afVelocityY[i] > 0.0f ? WvAnimState::ANIM_JUMP
											  : WvAnimState::ANIM_FALL;
	}

	if (ePrevious == WvAnimState::ANIM_JUMP ||
		ePrevious == WvAnimState::ANIM_FALL)
	{
		return WvAnimState::ANIM_LAND;
	}

	if (ePrevious == WvAnimState::ANIM_LAND &&
		sWorld.m_afAnimTime[i] < ANIMATION_LAND_SECONDS)
	{
		return WvAnimState::ANIM_LAND;
	}

	return std::fabs(sWorld.m_afVelocityX[i]) > ANIMATION_RUN_THRESHOLD
			 ? WvAnimState::ANIM_RUN
			 : WvAnimState::ANIM_IDLE;
}

void WvAnimationSystem::Update(WvWorld& sWorld, const float fStep)
{
	for (uint32_t i = 0; i < WORLD_PLAYER_COUNT; i++)
	{
		const WvAnimState eState = ChooseState(sWorld, i);

		if (eState != sWorld.m_aeAnimState[i])
		{
			sWorld.m_aeAnimState[i] = eState;
			sWorld.m_afAnimTime[i] = 0.0f;
		}
		else
		{
			sWorld.m_afAnimTime[i] += fStep;
		}

		sWorld.m_afRunPhase[i] += std::fabs(sWorld.m_afVelocityX[i]) * fStep;

		float fTargetX = 1.0f;
		float fTargetY = 1.0f;

		switch (eState)
		{
		case WvAnimState::ANIM_RUN:
			fTargetY = 1.0f + 0.06f * std::sin(sWorld.m_afRunPhase[i] * 3.0f);
			fTargetX = 2.0f - fTargetY;
			break;
		case WvAnimState::ANIM_JUMP:
			fTargetX = 0.8f;
			fTargetY = 1.25f;
			break;
		case WvAnimState::ANIM_FALL:
			fTargetX = 0.9f;
			fTargetY = 1.1f;
			break;
		case WvAnimState::ANIM_LAND:
			fTargetX = 1.35f;
			fTargetY = 0.65f;
			break;
		case WvAnimState::ANIM_IDLE:
			fTargetY = 1.0f + 0.02f * std::sin(sWorld.m_afAnimTime[i] * 4.0f);
			fTargetX = 2.0f - fTargetY;
			break;
		}

		// Exponential approach, rate-independent of the frame step.
		const float fBlend = 1.0f - std::exp(-ANIMATION_BLEND_RATE * fStep);
		sWorld.m_afScaleX[i] += (fTargetX - sWorld.m_afScaleX[i]) * fBlend;
		sWorld.m_afScaleY[i] += (fTargetY - sWorld.m_afScaleY[i]) * fBlend;
	}
}

} // namespace weev
