#include "input/InputFrameBuilder.h"

#include "core/Assert.h"

#include <cstring>

namespace weev
{

WvInputFrameBuilder::WvInputFrameBuilder()
{
	Reset();
}

void WvInputFrameBuilder::Reset()
{
	// memset, not assignment: the hash reads every byte, reserved ones
	// included, so both buffers start as all zeroes.
	std::memset((void*)m_aoFrames, 0, sizeof(m_aoFrames));
	std::memset(m_aabyKeyHolders, 0, sizeof(m_aabyKeyHolders));
	std::memset(m_abyMouseHolders, 0, sizeof(m_abyMouseHolders));
	std::memset(m_aauwPadHolders, 0, sizeof(m_aauwPadHolders));
	m_udwFront = 0;
}

const WvInputFrame& WvInputFrameBuilder::GetFront() const
{
	return m_aoFrames[m_udwFront];
}

const WvInputFrame& WvInputFrameBuilder::Build(
	const uint64_t uqwFrameIndex,
	const uint64_t uqwStepMicroseconds,
	const WvInputEvent* const psEvents,
	const uint32_t udwEventCount
)
{
	const uint32_t udwBack = 1 - m_udwFront;
	WvInputFrame& oBack = m_aoFrames[udwBack];

	// The back buffer still holds the frame before last. Start it from
	// the front, so state that persists (positions, pads, activity)
	// carries on, then clear what belongs to one frame only.
	oBack = m_aoFrames[m_udwFront];
	BeginFrame(oBack, uqwFrameIndex, uqwStepMicroseconds);

	for (uint32_t i = 0; i < udwEventCount; i++)
	{
		Apply(oBack, psEvents[i]);
	}

	m_udwFront = udwBack;
	return oBack;
}

void WvInputFrameBuilder::BeginFrame(
	WvInputFrame& oFrame,
	const uint64_t uqwFrameIndex,
	const uint64_t uqwStepMicroseconds
) const
{
	oFrame.m_uqwFrameIndex = uqwFrameIndex;
	oFrame.m_uqwStepMicroseconds = uqwStepMicroseconds;

	std::memset(oFrame.m_asKeyEdges, 0, sizeof(oFrame.m_asKeyEdges));
	oFrame.m_byKeyEdgeCount = 0;
	oFrame.m_byKeyEdgeOverflow = 0;
	oFrame.m_byFocusLost = 0;

	oFrame.m_dwMouseDeltaX = 0;
	oFrame.m_dwMouseDeltaY = 0;
	oFrame.m_dwWheel = 0;
	oFrame.m_dwWheelX = 0;
	std::memset(oFrame.m_abyMousePressCount, 0, INPUT_MOUSE_BUTTON_COUNT);
	std::memset(oFrame.m_abyMouseReleaseCount, 0, INPUT_MOUSE_BUTTON_COUNT);

	for (uint32_t i = 0; i < INPUT_GAMEPAD_SLOT_COUNT; i++)
	{
		WvInputPadState& sPad = oFrame.m_asPads[i];
		std::memset(sPad.m_abyPressCount, 0, sizeof(sPad.m_abyPressCount));
		std::memset(sPad.m_abyReleaseCount, 0, sizeof(sPad.m_abyReleaseCount));
	}
}

void WvInputFrameBuilder::Apply(
	WvInputFrame& oFrame, const WvInputEvent& sEvent
)
{
	const bool bDown = sEvent.m_adwValues[0] != 0;

	switch (sEvent.m_eType)
	{
	case WvInputEventType::EVENT_BUTTON:
		if (sEvent.m_eDevice == WvInputDevice::DEVICE_KEYBOARD)
		{
			ApplyKey(
				oFrame,
				WvKey(sEvent.m_uwControl),
				sEvent.m_eSource,
				sEvent.m_byIndex,
				bDown
			);
		}
		else if (sEvent.m_eDevice == WvInputDevice::DEVICE_MOUSE)
		{
			ApplyMouseButton(
				oFrame, sEvent.m_uwControl, sEvent.m_eSource, bDown
			);
		}
		else if (sEvent.m_eDevice == WvInputDevice::DEVICE_GAMEPAD)
		{
			ApplyPadButton(
				oFrame,
				sEvent.m_byIndex,
				sEvent.m_uwControl,
				sEvent.m_eSource,
				bDown
			);
		}
		break;

	case WvInputEventType::EVENT_AXIS:
		if (sEvent.m_byIndex < INPUT_GAMEPAD_SLOT_COUNT &&
			sEvent.m_uwControl < uint32_t(WvGamepadAxis::AXIS_COUNT))
		{
			oFrame.m_asPads[sEvent.m_byIndex].m_awAxes[sEvent.m_uwControl] =
				int16_t(sEvent.m_adwValues[0]); // intentional narrowing
		}
		break;

	case WvInputEventType::EVENT_MOTION:
		if (sEvent.m_uwControl == uint16_t(WvMouseMotion::MOTION_MOVE))
		{
			oFrame.m_dwMouseDeltaX += sEvent.m_adwValues[0];
			oFrame.m_dwMouseDeltaY += sEvent.m_adwValues[1];
		}
		else if (sEvent.m_uwControl == uint16_t(WvMouseMotion::MOTION_WHEEL))
		{
			oFrame.m_dwWheel += sEvent.m_adwValues[0];
		}
		else
		{
			oFrame.m_dwWheelX += sEvent.m_adwValues[0];
		}
		break;

	case WvInputEventType::EVENT_POSITION:
		oFrame.m_dwMouseX = sEvent.m_adwValues[0];
		oFrame.m_dwMouseY = sEvent.m_adwValues[1];
		break;

	case WvInputEventType::EVENT_CONNECT:
		if (sEvent.m_byIndex < INPUT_GAMEPAD_SLOT_COUNT)
		{
			WvInputPadState& sPad = oFrame.m_asPads[sEvent.m_byIndex];
			sPad.m_byConnected = 1;
			sPad.m_udwStableId = uint32_t(sEvent.m_adwValues[0]);
			sPad.m_eFamily = WvGamepadFamily(sEvent.m_adwValues[1]);
			sPad.m_udwPlatformUser = uint32_t(sEvent.m_adwValues[2]);
			sPad.m_udwGeneration++;
		}
		break;

	case WvInputEventType::EVENT_DISCONNECT:
		if (sEvent.m_byIndex < INPUT_GAMEPAD_SLOT_COUNT)
		{
			// The slot keeps its stable id, so the backend can hand the
			// same device the same slot when it comes back.
			ReleasePad(oFrame, sEvent.m_byIndex);
			oFrame.m_asPads[sEvent.m_byIndex].m_byConnected = 0;
		}
		break;

	case WvInputEventType::EVENT_FOCUS_LOST:
		ReleaseSource(oFrame, sEvent.m_eSource);
		oFrame.m_byFocusLost = 1;
		break;

	case WvInputEventType::EVENT_ACTIVITY:
		oFrame.m_eActivity = WvInputActivity(sEvent.m_adwValues[0]);
		break;

	case WvInputEventType::EVENT_VIEWPORT:
		oFrame.m_dwViewportWidth = sEvent.m_adwValues[0];
		oFrame.m_dwViewportHeight = sEvent.m_adwValues[1];
		oFrame.m_dwViewportScalePercent = sEvent.m_adwValues[2];
		break;
	}
}

bool WvInputFrameBuilder::IsKeyHeld(const uint32_t udwKey) const
{
	for (uint32_t s = 0; s < SOURCE_COUNT; s++)
	{
		if (m_aabyKeyHolders[s][udwKey] != 0)
		{
			return true;
		}
	}

	return false;
}

bool WvInputFrameBuilder::IsMouseHeld(const uint32_t udwButton) const
{
	for (uint32_t s = 0; s < SOURCE_COUNT; s++)
	{
		if ((m_abyMouseHolders[s] & (1u << udwButton)) != 0)
		{
			return true;
		}
	}

	return false;
}

bool WvInputFrameBuilder::IsPadHeld(
	const uint32_t udwSlot, const uint32_t udwButton
) const
{
	for (uint32_t s = 0; s < SOURCE_COUNT; s++)
	{
		if ((m_aauwPadHolders[s][udwSlot] & (1u << udwButton)) != 0)
		{
			return true;
		}
	}

	return false;
}

void WvInputFrameBuilder::ApplyKey(
	WvInputFrame& oFrame,
	const WvKey eKey,
	const WvInputSource eSource,
	const uint8_t byKeyboard,
	const bool bDown
)
{
	const uint32_t udwKey = uint32_t(eKey);
	const uint32_t udwSource = uint32_t(eSource);

	if (udwKey == 0 || udwSource >= SOURCE_COUNT ||
		byKeyboard >= INPUT_KEYBOARD_COUNT_MAX)
	{
		return;
	}

	const bool bWasHeld = IsKeyHeld(udwKey);
	uint8_t& byHolders = m_aabyKeyHolders[udwSource][udwKey];
	const uint8_t byBit = uint8_t(1u << byKeyboard);

	if (bDown)
	{
		byHolders |= byBit;
	}
	else
	{
		byHolders &= uint8_t(~byBit);
	}

	const bool bIsHeld = IsKeyHeld(udwKey);

	if (bWasHeld != bIsHeld)
	{
		AddKeyEdge(oFrame, udwKey, bIsHeld);
		SetKeyBit(oFrame, udwKey, bIsHeld);
	}
}

void WvInputFrameBuilder::ApplyMouseButton(
	WvInputFrame& oFrame,
	const uint32_t udwButton,
	const WvInputSource eSource,
	const bool bDown
)
{
	const uint32_t udwSource = uint32_t(eSource);

	if (udwButton >= INPUT_MOUSE_BUTTON_COUNT || udwSource >= SOURCE_COUNT)
	{
		return;
	}

	const bool bWasHeld = IsMouseHeld(udwButton);
	const uint8_t byBit = uint8_t(1u << udwButton);

	if (bDown)
	{
		m_abyMouseHolders[udwSource] |= byBit;
	}
	else
	{
		m_abyMouseHolders[udwSource] &= uint8_t(~byBit);
	}

	const bool bIsHeld = IsMouseHeld(udwButton);

	if (bWasHeld == bIsHeld)
	{
		return;
	}

	if (bIsHeld)
	{
		AddSaturating(oFrame.m_abyMousePressCount[udwButton]);
		oFrame.m_byMouseButtonsDown |= byBit;
	}
	else
	{
		AddSaturating(oFrame.m_abyMouseReleaseCount[udwButton]);
		oFrame.m_byMouseButtonsDown &= uint8_t(~byBit);
	}
}

void WvInputFrameBuilder::ApplyPadButton(
	WvInputFrame& oFrame,
	const uint32_t udwSlot,
	const uint32_t udwButton,
	const WvInputSource eSource,
	const bool bDown
)
{
	const uint32_t udwSource = uint32_t(eSource);

	if (udwSlot >= INPUT_GAMEPAD_SLOT_COUNT ||
		udwButton >= uint32_t(WvGamepadButton::BUTTON_COUNT) ||
		udwSource >= SOURCE_COUNT)
	{
		return;
	}

	const bool bWasHeld = IsPadHeld(udwSlot, udwButton);
	const uint16_t uwBit = uint16_t(1u << udwButton);

	if (bDown)
	{
		m_aauwPadHolders[udwSource][udwSlot] |= uwBit;
	}
	else
	{
		m_aauwPadHolders[udwSource][udwSlot] &= uint16_t(~uwBit);
	}

	const bool bIsHeld = IsPadHeld(udwSlot, udwButton);

	if (bWasHeld == bIsHeld)
	{
		return;
	}

	WvInputPadState& sPad = oFrame.m_asPads[udwSlot];

	if (bIsHeld)
	{
		AddSaturating(sPad.m_abyPressCount[udwButton]);
		sPad.m_uwButtonsDown |= uwBit;
	}
	else
	{
		AddSaturating(sPad.m_abyReleaseCount[udwButton]);
		sPad.m_uwButtonsDown &= uint16_t(~uwBit);
	}
}

void WvInputFrameBuilder::ReleaseSource(
	WvInputFrame& oFrame, const WvInputSource eSource
)
{
	const uint32_t udwSource = uint32_t(eSource);

	if (udwSource >= SOURCE_COUNT)
	{
		return;
	}

	for (uint32_t udwKey = 0; udwKey < INPUT_KEY_COUNT; udwKey++)
	{
		if (m_aabyKeyHolders[udwSource][udwKey] == 0)
		{
			continue;
		}

		m_aabyKeyHolders[udwSource][udwKey] = 0;

		if (!IsKeyHeld(udwKey))
		{
			AddKeyEdge(oFrame, udwKey, false);
			SetKeyBit(oFrame, udwKey, false);
		}
	}

	for (uint32_t i = 0; i < INPUT_MOUSE_BUTTON_COUNT; i++)
	{
		if ((m_abyMouseHolders[udwSource] & (1u << i)) != 0)
		{
			ApplyMouseButton(oFrame, i, eSource, false);
		}
	}

	for (uint32_t udwSlot = 0; udwSlot < INPUT_GAMEPAD_SLOT_COUNT; udwSlot++)
	{
		for (uint32_t i = 0; i < uint32_t(WvGamepadButton::BUTTON_COUNT); i++)
		{
			if ((m_aauwPadHolders[udwSource][udwSlot] & (1u << i)) != 0)
			{
				ApplyPadButton(oFrame, udwSlot, i, eSource, false);
			}
		}
	}
}

void WvInputFrameBuilder::ReleasePad(
	WvInputFrame& oFrame, const uint32_t udwSlot
)
{
	for (uint32_t s = 0; s < SOURCE_COUNT; s++)
	{
		for (uint32_t i = 0; i < uint32_t(WvGamepadButton::BUTTON_COUNT); i++)
		{
			if ((m_aauwPadHolders[s][udwSlot] & (1u << i)) != 0)
			{
				ApplyPadButton(oFrame, udwSlot, i, WvInputSource(s), false);
			}
		}
	}

	std::memset(
		oFrame.m_asPads[udwSlot].m_awAxes,
		0,
		sizeof(oFrame.m_asPads[udwSlot].m_awAxes)
	);
}

void WvInputFrameBuilder::AddKeyEdge(
	WvInputFrame& oFrame, const uint32_t udwKey, const bool bPress
)
{
	WvInputKeyEdge* psEdge = nullptr;

	for (uint32_t i = 0; i < oFrame.m_byKeyEdgeCount; i++)
	{
		if (oFrame.m_asKeyEdges[i].m_byKey == udwKey)
		{
			psEdge = &oFrame.m_asKeyEdges[i];
			break;
		}
	}

	if (psEdge == nullptr)
	{
		if (oFrame.m_byKeyEdgeCount >= WvInputFrame::KEY_EDGE_CAPACITY)
		{
			oFrame.m_byKeyEdgeOverflow = 1;
			return;
		}

		psEdge = &oFrame.m_asKeyEdges[oFrame.m_byKeyEdgeCount];
		psEdge->m_byKey = uint8_t(udwKey);
		oFrame.m_byKeyEdgeCount++;
	}

	AddSaturating(bPress ? psEdge->m_byPressCount : psEdge->m_byReleaseCount);
}

void WvInputFrameBuilder::SetKeyBit(
	WvInputFrame& oFrame, const uint32_t udwKey, const bool bDown
)
{
	const uint32_t udwBit = 1u << (udwKey % 32);

	if (bDown)
	{
		oFrame.m_audwKeysDown[udwKey / 32] |= udwBit;
	}
	else
	{
		oFrame.m_audwKeysDown[udwKey / 32] &= ~udwBit;
	}
}

void WvInputFrameBuilder::AddSaturating(uint8_t& byCount)
{
	if (byCount < 0xFF)
	{
		byCount++;
	}
}

} // namespace weev
