#pragma once

// The immutable snapshot one frame reads: every device's state, plus the
// presses and releases that happened since the last frame.
//
// A tap that goes down and up inside one frame reads as one press and
// one release with IsDown false. Check GetPressCount, not only IsDown,
// or fast taps are missed.
//
// The layout is bit-defined: fixed field order, no padding (asserted),
// so Hash() reads only real data and two platforms that saw the same
// input produce the same hash.

#include "input/InputEvent.h"

#include <type_traits>

namespace weev
{

// A control named without its value, as rebinding screens need.
struct WvInputControl
{
	WvInputDevice m_eDevice = WvInputDevice::DEVICE_KEYBOARD;
	uint8_t m_byIndex = 0;
	uint16_t m_uwControl = 0;
};

struct WvInputKeyEdge
{
	uint8_t m_byKey;
	uint8_t m_byPressCount;
	uint8_t m_byReleaseCount;
	uint8_t m_byReserved;
};

struct WvInputPadState
{
	uint32_t m_udwStableId;
	uint32_t m_udwGeneration;
	uint32_t m_udwPlatformUser;
	int16_t m_awAxes[uint32_t(WvGamepadAxis::AXIS_COUNT)];
	uint16_t m_uwButtonsDown;
	uint8_t m_abyPressCount[uint32_t(WvGamepadButton::BUTTON_COUNT)];
	uint8_t m_abyReleaseCount[uint32_t(WvGamepadButton::BUTTON_COUNT)];
	uint8_t m_byConnected;
	WvGamepadFamily m_eFamily;
	uint8_t m_abyReserved[2];
};

class WvInputFrame
{
public:
	// Distinct keys with an edge in one frame. Past this, down state is
	// still exact but the extra keys' counts are lost, and flagged.
	static const uint32_t KEY_EDGE_CAPACITY = 32;

	uint64_t GetFrameIndex() const;
	uint64_t GetStepMicroseconds() const;

	bool IsDown(WvKey eKey) const;
	uint32_t GetPressCount(WvKey eKey) const;
	uint32_t GetReleaseCount(WvKey eKey) const;
	uint32_t GetKeyEdgeCount() const;
	const WvInputKeyEdge& GetKeyEdge(uint32_t udwIndex) const;
	bool HasKeyEdgeOverflow() const;

	bool IsMouseDown(WvMouseButton eButton) const;
	uint32_t GetMousePressCount(WvMouseButton eButton) const;
	uint32_t GetMouseReleaseCount(WvMouseButton eButton) const;

	// In 1/16 logical px.
	void GetMousePosition(int32_t& dwOutX, int32_t& dwOutY) const;
	void GetMouseDelta(int32_t& dwOutX, int32_t& dwOutY) const;

	// In wheel units; INPUT_WHEEL_NOTCH per notch.
	int32_t GetWheel() const;

	bool IsGamepadConnected(uint32_t udwSlot) const;
	const WvInputPadState& GetGamepad(uint32_t udwSlot) const;
	bool IsGamepadDown(uint32_t udwSlot, WvGamepadButton eButton) const;
	uint32_t GetGamepadPressCount(
		uint32_t udwSlot, WvGamepadButton eButton
	) const;
	uint32_t GetGamepadReleaseCount(
		uint32_t udwSlot, WvGamepadButton eButton
	) const;
	int16_t GetGamepadAxis(uint32_t udwSlot, WvGamepadAxis eAxis) const;

	// One fixed formula, so every platform converts the same int16 to
	// the same float.
	float GetGamepadAxisFloat(uint32_t udwSlot, WvGamepadAxis eAxis) const;

	WvInputActivity GetActivity() const;
	bool IsFocusLostThisFrame() const;
	void GetViewport(int32_t& dwOutWidth, int32_t& dwOutHeight) const;

	// The first control pressed this frame, keys first. False if none.
	bool GetFirstPressedControl(WvInputControl& sOutControl) const;

	uint64_t Hash() const;

private:
	friend class WvInputFrameBuilder;

	const WvInputKeyEdge* FindKeyEdge(WvKey eKey) const;

	uint64_t m_uqwFrameIndex;
	uint64_t m_uqwStepMicroseconds;
	uint32_t m_audwKeysDown[INPUT_KEY_COUNT / 32];
	WvInputKeyEdge m_asKeyEdges[KEY_EDGE_CAPACITY];
	int32_t m_dwMouseX;
	int32_t m_dwMouseY;
	int32_t m_dwMouseDeltaX;
	int32_t m_dwMouseDeltaY;
	int32_t m_dwWheel;
	int32_t m_dwWheelX;
	int32_t m_dwViewportWidth;
	int32_t m_dwViewportHeight;
	int32_t m_dwViewportScalePercent;
	WvInputPadState m_asPads[INPUT_GAMEPAD_SLOT_COUNT];
	uint8_t m_abyMousePressCount[INPUT_MOUSE_BUTTON_COUNT];
	uint8_t m_abyMouseReleaseCount[INPUT_MOUSE_BUTTON_COUNT];
	uint8_t m_byMouseButtonsDown;
	uint8_t m_byKeyEdgeCount;
	WvInputActivity m_eActivity;
	uint8_t m_byFocusLost;
	uint8_t m_byKeyEdgeOverflow;
	uint8_t m_abyReserved[5];
};

static_assert(
	std::has_unique_object_representations_v<WvInputFrame>,
	"WvInputFrame must have no padding, or Hash() reads garbage"
);
static_assert(std::is_trivially_copyable_v<WvInputFrame>);

} // namespace weev
