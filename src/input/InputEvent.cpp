#include "input/InputEvent.h"

namespace weev
{

WvInputEvent WvInputEvent::MakeKey(
	const WvKey eKey,
	const bool bDown,
	const WvInputSource eSource,
	const uint8_t byKeyboard
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_KEYBOARD;
	sEvent.m_eType = WvInputEventType::EVENT_BUTTON;
	sEvent.m_uwControl = uint16_t(eKey);
	sEvent.m_adwValues[0] = bDown ? 1 : 0;
	sEvent.m_eSource = eSource;
	sEvent.m_byIndex = byKeyboard;
	return sEvent;
}

WvInputEvent WvInputEvent::MakeMouseButton(
	const WvMouseButton eButton, const bool bDown, const WvInputSource eSource
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_MOUSE;
	sEvent.m_eType = WvInputEventType::EVENT_BUTTON;
	sEvent.m_uwControl = uint16_t(eButton);
	sEvent.m_adwValues[0] = bDown ? 1 : 0;
	sEvent.m_eSource = eSource;
	return sEvent;
}

WvInputEvent WvInputEvent::MakePadButton(
	const uint8_t bySlot,
	const WvGamepadButton eButton,
	const bool bDown,
	const WvInputSource eSource
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_GAMEPAD;
	sEvent.m_eType = WvInputEventType::EVENT_BUTTON;
	sEvent.m_uwControl = uint16_t(eButton);
	sEvent.m_adwValues[0] = bDown ? 1 : 0;
	sEvent.m_eSource = eSource;
	sEvent.m_byIndex = bySlot;
	return sEvent;
}

WvInputEvent WvInputEvent::MakePadAxis(
	const uint8_t bySlot,
	const WvGamepadAxis eAxis,
	const int16_t wValue,
	const WvInputSource eSource
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_GAMEPAD;
	sEvent.m_eType = WvInputEventType::EVENT_AXIS;
	sEvent.m_uwControl = uint16_t(eAxis);
	sEvent.m_adwValues[0] = wValue;
	sEvent.m_eSource = eSource;
	sEvent.m_byIndex = bySlot;
	return sEvent;
}

WvInputEvent WvInputEvent::MakeMouseMotion(
	const WvMouseMotion eMotion,
	const int32_t dwX,
	const int32_t dwY,
	const WvInputSource eSource
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_MOUSE;
	sEvent.m_eType = WvInputEventType::EVENT_MOTION;
	sEvent.m_uwControl = uint16_t(eMotion);
	sEvent.m_adwValues[0] = dwX;
	sEvent.m_adwValues[1] = dwY;
	sEvent.m_eSource = eSource;
	return sEvent;
}

WvInputEvent WvInputEvent::MakeMousePosition(
	const int32_t dwX, const int32_t dwY, const WvInputSource eSource
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_MOUSE;
	sEvent.m_eType = WvInputEventType::EVENT_POSITION;
	sEvent.m_adwValues[0] = dwX;
	sEvent.m_adwValues[1] = dwY;
	sEvent.m_eSource = eSource;
	return sEvent;
}

WvInputEvent WvInputEvent::MakeConnect(
	const uint8_t bySlot,
	const uint32_t udwStableId,
	const WvGamepadFamily eFamily,
	const uint32_t udwPlatformUser
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_GAMEPAD;
	sEvent.m_eType = WvInputEventType::EVENT_CONNECT;
	sEvent.m_byIndex = bySlot;
	sEvent.m_adwValues[0] = int32_t(udwStableId);
	sEvent.m_adwValues[1] = int32_t(eFamily);
	sEvent.m_adwValues[2] = int32_t(udwPlatformUser);
	return sEvent;
}

WvInputEvent WvInputEvent::MakeDisconnect(const uint8_t bySlot)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_GAMEPAD;
	sEvent.m_eType = WvInputEventType::EVENT_DISCONNECT;
	sEvent.m_byIndex = bySlot;
	return sEvent;
}

WvInputEvent WvInputEvent::MakeFocusLost(const WvInputSource eSource)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_WINDOW;
	sEvent.m_eType = WvInputEventType::EVENT_FOCUS_LOST;
	sEvent.m_eSource = eSource;
	return sEvent;
}

WvInputEvent WvInputEvent::MakeActivity(const WvInputActivity eActivity)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_WINDOW;
	sEvent.m_eType = WvInputEventType::EVENT_ACTIVITY;
	sEvent.m_adwValues[0] = int32_t(eActivity);
	return sEvent;
}

WvInputEvent WvInputEvent::MakeViewport(
	const int32_t dwWidth, const int32_t dwHeight, const int32_t dwScalePercent
)
{
	WvInputEvent sEvent;
	sEvent.m_eDevice = WvInputDevice::DEVICE_WINDOW;
	sEvent.m_eType = WvInputEventType::EVENT_VIEWPORT;
	sEvent.m_adwValues[0] = dwWidth;
	sEvent.m_adwValues[1] = dwHeight;
	sEvent.m_adwValues[2] = dwScalePercent;
	return sEvent;
}

} // namespace weev
