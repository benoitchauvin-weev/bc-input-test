#include "game/Game.h"

#include "core/Hash.h"
#include "timer/Timer.h"

namespace weev
{

void WvGame::Init()
{
	m_sWorld = {};

	for (uint32_t i = 0; i < WORLD_PLAYER_COUNT; i++)
	{
		m_sWorld.m_afPositionX[i] = i == 0 ? -3.0f : 3.0f;
		m_sWorld.m_abyGrounded[i] = 1;
		m_sWorld.m_afScaleX[i] = 1.0f;
		m_sWorld.m_afScaleY[i] = 1.0f;
	}
}

void WvGame::Update(const WvTimer& oTimer, const WvInputFrame& oInput)
{
	const float fStep = float(oTimer.GetTimeStep());

	if (oInput.GetPressCount(WvKey::KEY_R) > 0)
	{
		Init();
	}

	m_oGameplay.Update(m_sWorld, oInput, fStep);
	m_oPhysics.Update(m_sWorld, fStep);
	m_oAnimation.Update(m_sWorld, fStep);
}

uint64_t WvGame::GetStateHash() const
{
	// Array by array rather than the whole struct, which has padding
	// between its byte arrays and its float arrays.
	const WvWorld& s = m_sWorld;
	uint64_t uqwHash = WvHash::HASH_SEED;

	uqwHash =
		WvHash::Fnv1a(s.m_afMoveIntent, sizeof(s.m_afMoveIntent), uqwHash);
	uqwHash =
		WvHash::Fnv1a(s.m_afJumpBuffer, sizeof(s.m_afJumpBuffer), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_abyJumpHeld, sizeof(s.m_abyJumpHeld), uqwHash);
	uqwHash =
		WvHash::Fnv1a(s.m_abyJumpRequest, sizeof(s.m_abyJumpRequest), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afPositionX, sizeof(s.m_afPositionX), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afPositionY, sizeof(s.m_afPositionY), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afVelocityX, sizeof(s.m_afVelocityX), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afVelocityY, sizeof(s.m_afVelocityY), uqwHash);
	uqwHash =
		WvHash::Fnv1a(s.m_afCoyoteTime, sizeof(s.m_afCoyoteTime), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_abyGrounded, sizeof(s.m_abyGrounded), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_abyJumpCut, sizeof(s.m_abyJumpCut), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_aeAnimState, sizeof(s.m_aeAnimState), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afAnimTime, sizeof(s.m_afAnimTime), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afRunPhase, sizeof(s.m_afRunPhase), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afScaleX, sizeof(s.m_afScaleX), uqwHash);
	uqwHash = WvHash::Fnv1a(s.m_afScaleY, sizeof(s.m_afScaleY), uqwHash);
	uqwHash =
		WvHash::Fnv1a(s.m_udwJumpCount, sizeof(s.m_udwJumpCount), uqwHash);
	return uqwHash;
}

const WvWorld& WvGame::GetWorld() const
{
	return m_sWorld;
}

} // namespace weev
