#pragma once

// Reads input, writes intent: which way each player wants to move, and
// whether a jump should start. The only system that reads input.
//
// Reads: input frame, grounded, coyote time.
// Writes: move intent, jump buffer, jump held, jump request.

#include "game/World.h"
#include "input/InputFrame.h"

namespace weev
{

class WvGameplaySystem
{
public:
	void Update(WvWorld& sWorld, const WvInputFrame& oInput, float fStep);

private:
	struct Binding
	{
		WvKey m_eLeft;
		WvKey m_eRight;
		WvKey m_eJump;
		WvKey m_eJumpAlternate;
		uint32_t m_udwPadSlot;
	};

	static float ReadMove(const WvInputFrame& oInput, const Binding& sBinding);
	static uint32_t ReadJumpPresses(
		const WvInputFrame& oInput, const Binding& sBinding
	);
	static bool ReadJumpHeld(
		const WvInputFrame& oInput, const Binding& sBinding
	);

	static const Binding s_asBindings[WORLD_PLAYER_COUNT];
};

} // namespace weev
