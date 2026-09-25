#include "game/GameplaySystem.h"

#include <cmath>

namespace weev
{

// A jump pressed this long before landing still happens on landing.
static const float GAMEPLAY_JUMP_BUFFER_SECONDS = 0.12f;
static const float GAMEPLAY_STICK_DEAD_ZONE = 0.25f;

// Two players on one merged keyboard, each with a gamepad slot of its
// own. Keys are physical positions: on AZERTY, player 1 is still the
// keys where WASD sit on QWERTY.
const WvGameplaySystem::Binding
	WvGameplaySystem::s_asBindings[WORLD_PLAYER_COUNT] = {
		{WvKey::KEY_A, WvKey::KEY_D, WvKey::KEY_SPACE, WvKey::KEY_W, 0},
		{WvKey::KEY_LEFT,
		 WvKey::KEY_RIGHT,
		 WvKey::KEY_UP,
		 WvKey::KEY_RIGHT_CTRL,
		 1},
};

float WvGameplaySystem::ReadMove(
	const WvInputFrame& oInput, const Binding& sBinding
)
{
	float fMove = 0.0f;

	if (oInput.IsDown(sBinding.m_eLeft))
	{
		fMove -= 1.0f;
	}

	if (oInput.IsDown(sBinding.m_eRight))
	{
		fMove += 1.0f;
	}

	if (!oInput.IsGamepadConnected(sBinding.m_udwPadSlot))
	{
		return fMove;
	}

	const uint32_t udwSlot = sBinding.m_udwPadSlot;

	if (oInput.IsGamepadDown(udwSlot, WvGamepadButton::BUTTON_DPAD_LEFT))
	{
		fMove -= 1.0f;
	}

	if (oInput.IsGamepadDown(udwSlot, WvGamepadButton::BUTTON_DPAD_RIGHT))
	{
		fMove += 1.0f;
	}

	// Radial dead zone, rescaled so the stick still reaches 1.
	const float fStickX =
		oInput.GetGamepadAxisFloat(udwSlot, WvGamepadAxis::AXIS_LEFT_X);
	const float fStickY =
		oInput.GetGamepadAxisFloat(udwSlot, WvGamepadAxis::AXIS_LEFT_Y);
	const float fLength = std::sqrt(fStickX * fStickX + fStickY * fStickY);

	if (fLength > GAMEPLAY_STICK_DEAD_ZONE)
	{
		const float fScaled = (fLength - GAMEPLAY_STICK_DEAD_ZONE) /
							  (1.0f - GAMEPLAY_STICK_DEAD_ZONE) / fLength;
		fMove += fStickX * fScaled;
	}

	return fMove < -1.0f ? -1.0f : (fMove > 1.0f ? 1.0f : fMove);
}

uint32_t WvGameplaySystem::ReadJumpPresses(
	const WvInputFrame& oInput, const Binding& sBinding
)
{
	// Press counts, not IsDown: a tap that went down and up inside one
	// frame still jumps.
	uint32_t udwPresses = oInput.GetPressCount(sBinding.m_eJump) +
						  oInput.GetPressCount(sBinding.m_eJumpAlternate);

	if (oInput.IsGamepadConnected(sBinding.m_udwPadSlot))
	{
		udwPresses += oInput.GetGamepadPressCount(
			sBinding.m_udwPadSlot, WvGamepadButton::BUTTON_SOUTH
		);
	}

	return udwPresses;
}

bool WvGameplaySystem::ReadJumpHeld(
	const WvInputFrame& oInput, const Binding& sBinding
)
{
	if (oInput.IsDown(sBinding.m_eJump) ||
		oInput.IsDown(sBinding.m_eJumpAlternate))
	{
		return true;
	}

	return oInput.IsGamepadConnected(sBinding.m_udwPadSlot) &&
		   oInput.IsGamepadDown(
			   sBinding.m_udwPadSlot, WvGamepadButton::BUTTON_SOUTH
		   );
}

void WvGameplaySystem::Update(
	WvWorld& sWorld, const WvInputFrame& oInput, const float fStep
)
{
	for (uint32_t i = 0; i < WORLD_PLAYER_COUNT; i++)
	{
		const Binding& sBinding = s_asBindings[i];

		sWorld.m_afMoveIntent[i] = ReadMove(oInput, sBinding);
		sWorld.m_abyJumpHeld[i] = ReadJumpHeld(oInput, sBinding) ? 1 : 0;

		if (ReadJumpPresses(oInput, sBinding) > 0)
		{
			sWorld.m_afJumpBuffer[i] = GAMEPLAY_JUMP_BUFFER_SECONDS;
		}
		else if (sWorld.m_afJumpBuffer[i] > 0.0f)
		{
			sWorld.m_afJumpBuffer[i] -= fStep;
		}

		// Coyote time: a jump just after running off a ledge counts.
		const bool bCanJump = sWorld.m_abyGrounded[i] != 0 ||
							  sWorld.m_afCoyoteTime[i] > 0.0f;

		if (sWorld.m_afJumpBuffer[i] > 0.0f && bCanJump)
		{
			// Consumed here, so one buffered press is one jump.
			sWorld.m_abyJumpRequest[i] = 1;
			sWorld.m_afJumpBuffer[i] = 0.0f;
		}
	}
}

} // namespace weev
