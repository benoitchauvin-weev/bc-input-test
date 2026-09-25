#include "game/ScriptedAgent.h"

#include "input/Input.h"

namespace weev
{

struct WvAgentStep
{
	uint64_t m_uqwFrameOffset;
	WvKey m_eKey;
	bool m_bDown;
};

// Run right, jump twice, run left, jump once. The jumps are taps: down
// and up on the same frame, which only press counts can see.
static const WvAgentStep s_asAgentScript[] = {
	{30, WvKey::KEY_RIGHT, true},
	{60, WvKey::KEY_UP, true},
	{60, WvKey::KEY_UP, false},
	{110, WvKey::KEY_UP, true},
	{128, WvKey::KEY_UP, false},
	{150, WvKey::KEY_RIGHT, false},
	{190, WvKey::KEY_LEFT, true},
	{230, WvKey::KEY_UP, true},
	{230, WvKey::KEY_UP, false},
	{320, WvKey::KEY_LEFT, false},
};

void WvScriptedAgent::Update(const uint64_t uqwFrameIndex)
{
	if (uqwFrameIndex < m_uqwNextCycleFrame)
	{
		return;
	}

	ScheduleCycle(m_uqwNextCycleFrame);
	m_uqwNextCycleFrame += AGENT_CYCLE_FRAMES;
}

void WvScriptedAgent::ScheduleCycle(const uint64_t uqwStartFrame)
{
	const uint32_t udwStepCount = sizeof(s_asAgentScript) /
								  sizeof(s_asAgentScript[0]);

	for (uint32_t i = 0; i < udwStepCount; i++)
	{
		const WvAgentStep& sStep = s_asAgentScript[i];

		WvInput::PushAtFrame(
			WvInputEvent::MakeKey(
				sStep.m_eKey, sStep.m_bDown, WvInputSource::SOURCE_AGENT, 0
			),
			uqwStartFrame + sStep.m_uqwFrameOffset
		);
	}
}

} // namespace weev
