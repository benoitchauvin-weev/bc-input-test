#pragma once

// The canonical controls every backend translates into.
//
// Keys are USB HID usages (keyboard page 0x07): they name a physical
// position, not the letter printed on it, so WASD stays WASD on AZERTY.
// Gamepad buttons are named by position (south, east, west, north), not
// by vendor label.

#include <cstdint>

namespace weev
{

constexpr uint32_t INPUT_KEY_COUNT = 256;
constexpr uint32_t INPUT_MOUSE_BUTTON_COUNT = 5;
constexpr uint32_t INPUT_GAMEPAD_SLOT_COUNT = 8;
constexpr uint32_t INPUT_KEYBOARD_COUNT_MAX = 8;

// Mouse position is carried in 1/16 of a logical pixel.
constexpr int32_t INPUT_POSITION_SCALE = 16;

// One notch of a mouse wheel, as Windows reports it.
constexpr int32_t INPUT_WHEEL_NOTCH = 120;

enum class WvKey : uint8_t
{
	KEY_NONE = 0x00,

	KEY_A = 0x04,
	KEY_B = 0x05,
	KEY_C = 0x06,
	KEY_D = 0x07,
	KEY_E = 0x08,
	KEY_F = 0x09,
	KEY_G = 0x0A,
	KEY_H = 0x0B,
	KEY_I = 0x0C,
	KEY_J = 0x0D,
	KEY_K = 0x0E,
	KEY_L = 0x0F,
	KEY_M = 0x10,
	KEY_N = 0x11,
	KEY_O = 0x12,
	KEY_P = 0x13,
	KEY_Q = 0x14,
	KEY_R = 0x15,
	KEY_S = 0x16,
	KEY_T = 0x17,
	KEY_U = 0x18,
	KEY_V = 0x19,
	KEY_W = 0x1A,
	KEY_X = 0x1B,
	KEY_Y = 0x1C,
	KEY_Z = 0x1D,

	KEY_1 = 0x1E,
	KEY_2 = 0x1F,
	KEY_3 = 0x20,
	KEY_4 = 0x21,
	KEY_5 = 0x22,
	KEY_6 = 0x23,
	KEY_7 = 0x24,
	KEY_8 = 0x25,
	KEY_9 = 0x26,
	KEY_0 = 0x27,

	KEY_ENTER = 0x28,
	KEY_ESCAPE = 0x29,
	KEY_BACKSPACE = 0x2A,
	KEY_TAB = 0x2B,
	KEY_SPACE = 0x2C,
	KEY_MINUS = 0x2D,
	KEY_EQUALS = 0x2E,
	KEY_LEFT_BRACKET = 0x2F,
	KEY_RIGHT_BRACKET = 0x30,
	KEY_BACKSLASH = 0x31,
	KEY_NON_US_HASH = 0x32,
	KEY_SEMICOLON = 0x33,
	KEY_APOSTROPHE = 0x34,
	KEY_GRAVE = 0x35,
	KEY_COMMA = 0x36,
	KEY_PERIOD = 0x37,
	KEY_SLASH = 0x38,
	KEY_CAPS_LOCK = 0x39,

	KEY_F1 = 0x3A,
	KEY_F2 = 0x3B,
	KEY_F3 = 0x3C,
	KEY_F4 = 0x3D,
	KEY_F5 = 0x3E,
	KEY_F6 = 0x3F,
	KEY_F7 = 0x40,
	KEY_F8 = 0x41,
	KEY_F9 = 0x42,
	KEY_F10 = 0x43,
	KEY_F11 = 0x44,
	KEY_F12 = 0x45,

	KEY_PRINT_SCREEN = 0x46,
	KEY_SCROLL_LOCK = 0x47,
	KEY_PAUSE = 0x48,
	KEY_INSERT = 0x49,
	KEY_HOME = 0x4A,
	KEY_PAGE_UP = 0x4B,
	KEY_DELETE = 0x4C,
	KEY_END = 0x4D,
	KEY_PAGE_DOWN = 0x4E,
	KEY_RIGHT = 0x4F,
	KEY_LEFT = 0x50,
	KEY_DOWN = 0x51,
	KEY_UP = 0x52,

	KEY_NUM_LOCK = 0x53,
	KEY_KP_DIVIDE = 0x54,
	KEY_KP_MULTIPLY = 0x55,
	KEY_KP_MINUS = 0x56,
	KEY_KP_PLUS = 0x57,
	KEY_KP_ENTER = 0x58,
	KEY_KP_1 = 0x59,
	KEY_KP_2 = 0x5A,
	KEY_KP_3 = 0x5B,
	KEY_KP_4 = 0x5C,
	KEY_KP_5 = 0x5D,
	KEY_KP_6 = 0x5E,
	KEY_KP_7 = 0x5F,
	KEY_KP_8 = 0x60,
	KEY_KP_9 = 0x61,
	KEY_KP_0 = 0x62,
	KEY_KP_PERIOD = 0x63,
	KEY_NON_US_BACKSLASH = 0x64,
	KEY_APPLICATION = 0x65,

	KEY_LEFT_CTRL = 0xE0,
	KEY_LEFT_SHIFT = 0xE1,
	KEY_LEFT_ALT = 0xE2,
	KEY_LEFT_GUI = 0xE3,
	KEY_RIGHT_CTRL = 0xE4,
	KEY_RIGHT_SHIFT = 0xE5,
	KEY_RIGHT_ALT = 0xE6,
	KEY_RIGHT_GUI = 0xE7,
};

enum class WvMouseButton : uint8_t
{
	BUTTON_LEFT,
	BUTTON_RIGHT,
	BUTTON_MIDDLE,
	BUTTON_X1,
	BUTTON_X2,
};

enum class WvGamepadButton : uint8_t
{
	BUTTON_SOUTH,
	BUTTON_EAST,
	BUTTON_WEST,
	BUTTON_NORTH,
	BUTTON_LEFT_SHOULDER,
	BUTTON_RIGHT_SHOULDER,
	BUTTON_BACK,
	BUTTON_START,
	BUTTON_LEFT_STICK,
	BUTTON_RIGHT_STICK,
	BUTTON_DPAD_UP,
	BUTTON_DPAD_DOWN,
	BUTTON_DPAD_LEFT,
	BUTTON_DPAD_RIGHT,
	BUTTON_GUIDE,

	BUTTON_COUNT,
};

// Sticks are -32767..32767 with +Y up; triggers are 0..32767.
enum class WvGamepadAxis : uint8_t
{
	AXIS_LEFT_X,
	AXIS_LEFT_Y,
	AXIS_RIGHT_X,
	AXIS_RIGHT_Y,
	AXIS_LEFT_TRIGGER,
	AXIS_RIGHT_TRIGGER,

	AXIS_COUNT,
};

enum class WvGamepadFamily : uint8_t
{
	FAMILY_UNKNOWN,
	FAMILY_XBOX,
	FAMILY_PLAYSTATION,
	FAMILY_SWITCH,
	FAMILY_GENERIC,
};

// Who produced an event. Focus loss releases only what a human holds, so
// an agent's held keys survive the window losing focus.
enum class WvInputSource : uint8_t
{
	SOURCE_HUMAN,
	SOURCE_AGENT,

	SOURCE_COUNT,
};

enum class WvInputActivity : uint8_t
{
	ACTIVITY_FOCUSED,
	ACTIVITY_UNFOCUSED,
	ACTIVITY_OVERLAY,
	ACTIVITY_SUSPENDED,
};

} // namespace weev
