#include "input/InputFrame.h"

#include "core/Assert.h"
#include "core/Hash.h"

namespace weev
{

static const WvInputKeyEdge s_sNoEdge = {};

uint64_t WvInputFrame::GetFrameIndex() const
{
	return m_uqwFrameIndex;
}

uint64_t WvInputFrame::GetStepMicroseconds() const
{
	return m_uqwStepMicroseconds;
}

bool WvInputFrame::IsDown(const WvKey eKey) const
{
	const uint32_t udwKey = uint32_t(eKey);
	return (m_audwKeysDown[udwKey / 32] & (1u << (udwKey % 32))) != 0;
}

const WvInputKeyEdge* WvInputFrame::FindKeyEdge(const WvKey eKey) const
{
	for (uint32_t i = 0; i < m_byKeyEdgeCount; i++)
	{
		if (m_asKeyEdges[i].m_byKey == uint8_t(eKey))
		{
			return &m_asKeyEdges[i];
		}
	}

	return nullptr;
}

uint32_t WvInputFrame::GetPressCount(const WvKey eKey) const
{
	const WvInputKeyEdge* const psEdge = FindKeyEdge(eKey);
	return psEdge != nullptr ? psEdge->m_byPressCount : 0;
}

uint32_t WvInputFrame::GetReleaseCount(const WvKey eKey) const
{
	const WvInputKeyEdge* const psEdge = FindKeyEdge(eKey);
	return psEdge != nullptr ? psEdge->m_byReleaseCount : 0;
}

uint32_t WvInputFrame::GetKeyEdgeCount() const
{
	return m_byKeyEdgeCount;
}

const WvInputKeyEdge& WvInputFrame::GetKeyEdge(const uint32_t udwIndex) const
{
	WV_ASSERT(udwIndex < m_byKeyEdgeCount, "key edge out of range");

	if (udwIndex >= m_byKeyEdgeCount)
	{
		return s_sNoEdge;
	}

	return m_asKeyEdges[udwIndex];
}

bool WvInputFrame::HasKeyEdgeOverflow() const
{
	return m_byKeyEdgeOverflow != 0;
}

bool WvInputFrame::IsMouseDown(const WvMouseButton eButton) const
{
	return (m_byMouseButtonsDown & (1u << uint32_t(eButton))) != 0;
}

uint32_t WvInputFrame::GetMousePressCount(const WvMouseButton eButton) const
{
	return m_abyMousePressCount[uint32_t(eButton)];
}

uint32_t WvInputFrame::GetMouseReleaseCount(const WvMouseButton eButton) const
{
	return m_abyMouseReleaseCount[uint32_t(eButton)];
}

void WvInputFrame::GetMousePosition(int32_t& dwOutX, int32_t& dwOutY) const
{
	dwOutX = m_dwMouseX;
	dwOutY = m_dwMouseY;
}

void WvInputFrame::GetMouseDelta(int32_t& dwOutX, int32_t& dwOutY) const
{
	dwOutX = m_dwMouseDeltaX;
	dwOutY = m_dwMouseDeltaY;
}

int32_t WvInputFrame::GetWheel() const
{
	return m_dwWheel;
}

bool WvInputFrame::IsGamepadConnected(const uint32_t udwSlot) const
{
	return udwSlot < INPUT_GAMEPAD_SLOT_COUNT &&
		   m_asPads[udwSlot].m_byConnected != 0;
}

const WvInputPadState& WvInputFrame::GetGamepad(const uint32_t udwSlot) const
{
	WV_ASSERT(udwSlot < INPUT_GAMEPAD_SLOT_COUNT, "gamepad slot out of range");
	return m_asPads[udwSlot < INPUT_GAMEPAD_SLOT_COUNT ? udwSlot : 0];
}

bool WvInputFrame::IsGamepadDown(
	const uint32_t udwSlot, const WvGamepadButton eButton
) const
{
	return (GetGamepad(udwSlot).m_uwButtonsDown & (1u << uint32_t(eButton))) !=
		   0;
}

uint32_t WvInputFrame::GetGamepadPressCount(
	const uint32_t udwSlot, const WvGamepadButton eButton
) const
{
	return GetGamepad(udwSlot).m_abyPressCount[uint32_t(eButton)];
}

uint32_t WvInputFrame::GetGamepadReleaseCount(
	const uint32_t udwSlot, const WvGamepadButton eButton
) const
{
	return GetGamepad(udwSlot).m_abyReleaseCount[uint32_t(eButton)];
}

int16_t WvInputFrame::GetGamepadAxis(
	const uint32_t udwSlot, const WvGamepadAxis eAxis
) const
{
	return GetGamepad(udwSlot).m_awAxes[uint32_t(eAxis)];
}

float WvInputFrame::GetGamepadAxisFloat(
	const uint32_t udwSlot, const WvGamepadAxis eAxis
) const
{
	// Division is correctly rounded in IEEE 754, so this is the same
	// float everywhere. -32768 clamps to -1.
	const float fValue = float(GetGamepadAxis(udwSlot, eAxis)) / 32767.0f;
	return fValue < -1.0f ? -1.0f : fValue;
}

WvInputActivity WvInputFrame::GetActivity() const
{
	return m_eActivity;
}

bool WvInputFrame::IsFocusLostThisFrame() const
{
	return m_byFocusLost != 0;
}

void WvInputFrame::GetViewport(int32_t& dwOutWidth, int32_t& dwOutHeight) const
{
	dwOutWidth = m_dwViewportWidth;
	dwOutHeight = m_dwViewportHeight;
}

bool WvInputFrame::GetFirstPressedControl(WvInputControl& sOutControl) const
{
	for (uint32_t i = 0; i < m_byKeyEdgeCount; i++)
	{
		if (m_asKeyEdges[i].m_byPressCount > 0)
		{
			sOutControl.m_eDevice = WvInputDevice::DEVICE_KEYBOARD;
			sOutControl.m_byIndex = 0;
			sOutControl.m_uwControl = m_asKeyEdges[i].m_byKey;
			return true;
		}
	}

	for (uint32_t i = 0; i < INPUT_MOUSE_BUTTON_COUNT; i++)
	{
		if (m_abyMousePressCount[i] > 0)
		{
			sOutControl.m_eDevice = WvInputDevice::DEVICE_MOUSE;
			sOutControl.m_byIndex = 0;
			sOutControl.m_uwControl = uint16_t(i);
			return true;
		}
	}

	for (uint32_t udwSlot = 0; udwSlot < INPUT_GAMEPAD_SLOT_COUNT; udwSlot++)
	{
		const WvInputPadState& sPad = m_asPads[udwSlot];

		for (uint32_t i = 0; i < uint32_t(WvGamepadButton::BUTTON_COUNT); i++)
		{
			if (sPad.m_abyPressCount[i] > 0)
			{
				sOutControl.m_eDevice = WvInputDevice::DEVICE_GAMEPAD;
				sOutControl.m_byIndex = uint8_t(udwSlot);
				sOutControl.m_uwControl = uint16_t(i);
				return true;
			}
		}
	}

	return false;
}

uint64_t WvInputFrame::Hash() const
{
	return WvHash::Fnv1a(this, sizeof(*this));
}

} // namespace weev
