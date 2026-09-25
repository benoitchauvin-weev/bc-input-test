#pragma once

// What a backend, a test or an agent pushes. Plain data, one shape for
// every device; the type says how to read the values.

#include "input/InputTypes.h"

namespace weev
{

enum class WvInputDevice : uint8_t
{
	DEVICE_KEYBOARD,
	DEVICE_MOUSE,
	DEVICE_GAMEPAD,
	DEVICE_WINDOW,
};

enum class WvInputEventType : uint8_t
{
	// Control pressed (value 1) or released (value 0).
	EVENT_BUTTON,

	// Absolute analog value; the latest in a frame wins.
	EVENT_AXIS,

	// Relative motion (mouse delta, wheel); summed over a frame.
	EVENT_MOTION,

	// Absolute pointer position (x, y) in 1/16 logical px; latest wins.
	EVENT_POSITION,

	// A gamepad slot gained a device: values are stable id, family and
	// platform user. The index is the slot.
	EVENT_CONNECT,
	EVENT_DISCONNECT,

	// Release everything this source holds.
	EVENT_FOCUS_LOST,

	// Value is a WvInputActivity.
	EVENT_ACTIVITY,

	// Values are width and height in logical px, and scale in percent.
	EVENT_VIEWPORT,
};

// Controls for DEVICE_MOUSE motion events.
enum class WvMouseMotion : uint8_t
{
	MOTION_MOVE,
	MOTION_WHEEL,
	MOTION_WHEEL_X,
};

struct WvInputEvent
{
	// Engine ticks at Push. Never recorded and never hashed: a frame is
	// identified by its index, events inside it by arrival order.
	uint64_t m_uqwTicks = 0;

	int32_t m_adwValues[3] = {};

	// A WvKey, WvMouseButton, WvGamepadButton, WvGamepadAxis or
	// WvMouseMotion, depending on the device and type.
	uint16_t m_uwControl = 0;

	WvInputDevice m_eDevice = WvInputDevice::DEVICE_KEYBOARD;
	WvInputEventType m_eType = WvInputEventType::EVENT_BUTTON;

	// Gamepad slot, or which keyboard or mouse sent it. Keyboards are
	// merged, but the index lets the builder count holders per key.
	uint8_t m_byIndex = 0;

	WvInputSource m_eSource = WvInputSource::SOURCE_HUMAN;
	uint8_t m_abyReserved[2] = {};

	static WvInputEvent MakeKey(
		WvKey eKey, bool bDown, WvInputSource eSource, uint8_t byKeyboard
	);
	static WvInputEvent MakeMouseButton(
		WvMouseButton eButton, bool bDown, WvInputSource eSource
	);
	static WvInputEvent MakePadButton(
		uint8_t bySlot,
		WvGamepadButton eButton,
		bool bDown,
		WvInputSource eSource
	);
	static WvInputEvent MakePadAxis(
		uint8_t bySlot,
		WvGamepadAxis eAxis,
		int16_t wValue,
		WvInputSource eSource
	);
	static WvInputEvent MakeMouseMotion(
		WvMouseMotion eMotion, int32_t dwX, int32_t dwY, WvInputSource eSource
	);
	static WvInputEvent MakeMousePosition(
		int32_t dwX, int32_t dwY, WvInputSource eSource
	);
	static WvInputEvent MakeConnect(
		uint8_t bySlot,
		uint32_t udwStableId,
		WvGamepadFamily eFamily,
		uint32_t udwPlatformUser
	);
	static WvInputEvent MakeDisconnect(uint8_t bySlot);
	static WvInputEvent MakeFocusLost(WvInputSource eSource);
	static WvInputEvent MakeActivity(WvInputActivity eActivity);
	static WvInputEvent MakeViewport(
		int32_t dwWidth, int32_t dwHeight, int32_t dwScalePercent
	);
};

static_assert(sizeof(WvInputEvent) == 32, "keep the event small");

} // namespace weev
