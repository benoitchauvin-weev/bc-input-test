#include "input/InputKeyMap.h"

#include <cstring>

namespace weev
{

// Windows virtual keys, repeated here so this file needs no windows.h.
static const uint32_t KEYMAP_VK_PAUSE = 0x13;
static const uint32_t KEYMAP_VK_NUMLOCK = 0x90;
static const uint32_t KEYMAP_VK_SNAPSHOT = 0x2C;

const WvInputKeyMap::KeyEntry WvInputKeyMap::s_asKeys[] = {
	{WvKey::KEY_A, "KeyA", "A"},
	{WvKey::KEY_B, "KeyB", "B"},
	{WvKey::KEY_C, "KeyC", "C"},
	{WvKey::KEY_D, "KeyD", "D"},
	{WvKey::KEY_E, "KeyE", "E"},
	{WvKey::KEY_F, "KeyF", "F"},
	{WvKey::KEY_G, "KeyG", "G"},
	{WvKey::KEY_H, "KeyH", "H"},
	{WvKey::KEY_I, "KeyI", "I"},
	{WvKey::KEY_J, "KeyJ", "J"},
	{WvKey::KEY_K, "KeyK", "K"},
	{WvKey::KEY_L, "KeyL", "L"},
	{WvKey::KEY_M, "KeyM", "M"},
	{WvKey::KEY_N, "KeyN", "N"},
	{WvKey::KEY_O, "KeyO", "O"},
	{WvKey::KEY_P, "KeyP", "P"},
	{WvKey::KEY_Q, "KeyQ", "Q"},
	{WvKey::KEY_R, "KeyR", "R"},
	{WvKey::KEY_S, "KeyS", "S"},
	{WvKey::KEY_T, "KeyT", "T"},
	{WvKey::KEY_U, "KeyU", "U"},
	{WvKey::KEY_V, "KeyV", "V"},
	{WvKey::KEY_W, "KeyW", "W"},
	{WvKey::KEY_X, "KeyX", "X"},
	{WvKey::KEY_Y, "KeyY", "Y"},
	{WvKey::KEY_Z, "KeyZ", "Z"},
	{WvKey::KEY_1, "Digit1", "1"},
	{WvKey::KEY_2, "Digit2", "2"},
	{WvKey::KEY_3, "Digit3", "3"},
	{WvKey::KEY_4, "Digit4", "4"},
	{WvKey::KEY_5, "Digit5", "5"},
	{WvKey::KEY_6, "Digit6", "6"},
	{WvKey::KEY_7, "Digit7", "7"},
	{WvKey::KEY_8, "Digit8", "8"},
	{WvKey::KEY_9, "Digit9", "9"},
	{WvKey::KEY_0, "Digit0", "0"},
	{WvKey::KEY_ENTER, "Enter", "Enter"},
	{WvKey::KEY_ESCAPE, "Escape", "Esc"},
	{WvKey::KEY_BACKSPACE, "Backspace", "Backspace"},
	{WvKey::KEY_TAB, "Tab", "Tab"},
	{WvKey::KEY_SPACE, "Space", "Space"},
	{WvKey::KEY_MINUS, "Minus", "-"},
	{WvKey::KEY_EQUALS, "Equal", "="},
	{WvKey::KEY_LEFT_BRACKET, "BracketLeft", "["},
	{WvKey::KEY_RIGHT_BRACKET, "BracketRight", "]"},
	{WvKey::KEY_BACKSLASH, "Backslash", "\\"},
	{WvKey::KEY_SEMICOLON, "Semicolon", ";"},
	{WvKey::KEY_APOSTROPHE, "Quote", "'"},
	{WvKey::KEY_GRAVE, "Backquote", "`"},
	{WvKey::KEY_COMMA, "Comma", ","},
	{WvKey::KEY_PERIOD, "Period", "."},
	{WvKey::KEY_SLASH, "Slash", "/"},
	{WvKey::KEY_CAPS_LOCK, "CapsLock", "CapsLock"},
	{WvKey::KEY_F1, "F1", "F1"},
	{WvKey::KEY_F2, "F2", "F2"},
	{WvKey::KEY_F3, "F3", "F3"},
	{WvKey::KEY_F4, "F4", "F4"},
	{WvKey::KEY_F5, "F5", "F5"},
	{WvKey::KEY_F6, "F6", "F6"},
	{WvKey::KEY_F7, "F7", "F7"},
	{WvKey::KEY_F8, "F8", "F8"},
	{WvKey::KEY_F9, "F9", "F9"},
	{WvKey::KEY_F10, "F10", "F10"},
	{WvKey::KEY_F11, "F11", "F11"},
	{WvKey::KEY_F12, "F12", "F12"},
	{WvKey::KEY_PRINT_SCREEN, "PrintScreen", "PrintScreen"},
	{WvKey::KEY_SCROLL_LOCK, "ScrollLock", "ScrollLock"},
	{WvKey::KEY_PAUSE, "Pause", "Pause"},
	{WvKey::KEY_INSERT, "Insert", "Insert"},
	{WvKey::KEY_HOME, "Home", "Home"},
	{WvKey::KEY_PAGE_UP, "PageUp", "PageUp"},
	{WvKey::KEY_DELETE, "Delete", "Delete"},
	{WvKey::KEY_END, "End", "End"},
	{WvKey::KEY_PAGE_DOWN, "PageDown", "PageDown"},
	{WvKey::KEY_RIGHT, "ArrowRight", "Right"},
	{WvKey::KEY_LEFT, "ArrowLeft", "Left"},
	{WvKey::KEY_DOWN, "ArrowDown", "Down"},
	{WvKey::KEY_UP, "ArrowUp", "Up"},
	{WvKey::KEY_NUM_LOCK, "NumLock", "NumLock"},
	{WvKey::KEY_KP_DIVIDE, "NumpadDivide", "KP/"},
	{WvKey::KEY_KP_MULTIPLY, "NumpadMultiply", "KP*"},
	{WvKey::KEY_KP_MINUS, "NumpadSubtract", "KP-"},
	{WvKey::KEY_KP_PLUS, "NumpadAdd", "KP+"},
	{WvKey::KEY_KP_ENTER, "NumpadEnter", "KPEnter"},
	{WvKey::KEY_KP_1, "Numpad1", "KP1"},
	{WvKey::KEY_KP_2, "Numpad2", "KP2"},
	{WvKey::KEY_KP_3, "Numpad3", "KP3"},
	{WvKey::KEY_KP_4, "Numpad4", "KP4"},
	{WvKey::KEY_KP_5, "Numpad5", "KP5"},
	{WvKey::KEY_KP_6, "Numpad6", "KP6"},
	{WvKey::KEY_KP_7, "Numpad7", "KP7"},
	{WvKey::KEY_KP_8, "Numpad8", "KP8"},
	{WvKey::KEY_KP_9, "Numpad9", "KP9"},
	{WvKey::KEY_KP_0, "Numpad0", "KP0"},
	{WvKey::KEY_KP_PERIOD, "NumpadDecimal", "KP."},
	{WvKey::KEY_NON_US_BACKSLASH, "IntlBackslash", "IntlBackslash"},
	{WvKey::KEY_APPLICATION, "ContextMenu", "Menu"},
	{WvKey::KEY_LEFT_CTRL, "ControlLeft", "LCtrl"},
	{WvKey::KEY_LEFT_SHIFT, "ShiftLeft", "LShift"},
	{WvKey::KEY_LEFT_ALT, "AltLeft", "LAlt"},
	{WvKey::KEY_LEFT_GUI, "MetaLeft", "LWin"},
	{WvKey::KEY_RIGHT_CTRL, "ControlRight", "RCtrl"},
	{WvKey::KEY_RIGHT_SHIFT, "ShiftRight", "RShift"},
	{WvKey::KEY_RIGHT_ALT, "AltRight", "RAlt"},
	{WvKey::KEY_RIGHT_GUI, "MetaRight", "RWin"},
};

const uint32_t WvInputKeyMap::s_udwKeyCount = sizeof(s_asKeys) /
											  sizeof(s_asKeys[0]);

WvKey WvInputKeyMap::FromWinScancode(
	const uint32_t udwScancode,
	const bool bExtended,
	const uint32_t udwVirtualKey
)
{
	// Pause arrives as 0x45 without the extended bit, NumLock as 0x45
	// with it, but only the virtual key is certain.
	if (udwVirtualKey == KEYMAP_VK_PAUSE)
	{
		return WvKey::KEY_PAUSE;
	}

	if (udwVirtualKey == KEYMAP_VK_NUMLOCK)
	{
		return WvKey::KEY_NUM_LOCK;
	}

	// PrintScreen is 0x37 extended, or 0x54 (SysRq) while Alt is held.
	if (udwVirtualKey == KEYMAP_VK_SNAPSHOT)
	{
		return WvKey::KEY_PRINT_SCREEN;
	}

	return bExtended ? FromWinExtendedScancode(udwScancode)
					 : FromWinBaseScancode(udwScancode);
}

WvKey WvInputKeyMap::FromWinBaseScancode(const uint32_t udwScancode)
{
	switch (udwScancode)
	{
	case 0x01:
		return WvKey::KEY_ESCAPE;
	case 0x02:
		return WvKey::KEY_1;
	case 0x03:
		return WvKey::KEY_2;
	case 0x04:
		return WvKey::KEY_3;
	case 0x05:
		return WvKey::KEY_4;
	case 0x06:
		return WvKey::KEY_5;
	case 0x07:
		return WvKey::KEY_6;
	case 0x08:
		return WvKey::KEY_7;
	case 0x09:
		return WvKey::KEY_8;
	case 0x0A:
		return WvKey::KEY_9;
	case 0x0B:
		return WvKey::KEY_0;
	case 0x0C:
		return WvKey::KEY_MINUS;
	case 0x0D:
		return WvKey::KEY_EQUALS;
	case 0x0E:
		return WvKey::KEY_BACKSPACE;
	case 0x0F:
		return WvKey::KEY_TAB;
	case 0x10:
		return WvKey::KEY_Q;
	case 0x11:
		return WvKey::KEY_W;
	case 0x12:
		return WvKey::KEY_E;
	case 0x13:
		return WvKey::KEY_R;
	case 0x14:
		return WvKey::KEY_T;
	case 0x15:
		return WvKey::KEY_Y;
	case 0x16:
		return WvKey::KEY_U;
	case 0x17:
		return WvKey::KEY_I;
	case 0x18:
		return WvKey::KEY_O;
	case 0x19:
		return WvKey::KEY_P;
	case 0x1A:
		return WvKey::KEY_LEFT_BRACKET;
	case 0x1B:
		return WvKey::KEY_RIGHT_BRACKET;
	case 0x1C:
		return WvKey::KEY_ENTER;
	case 0x1D:
		return WvKey::KEY_LEFT_CTRL;
	case 0x1E:
		return WvKey::KEY_A;
	case 0x1F:
		return WvKey::KEY_S;
	case 0x20:
		return WvKey::KEY_D;
	case 0x21:
		return WvKey::KEY_F;
	case 0x22:
		return WvKey::KEY_G;
	case 0x23:
		return WvKey::KEY_H;
	case 0x24:
		return WvKey::KEY_J;
	case 0x25:
		return WvKey::KEY_K;
	case 0x26:
		return WvKey::KEY_L;
	case 0x27:
		return WvKey::KEY_SEMICOLON;
	case 0x28:
		return WvKey::KEY_APOSTROPHE;
	case 0x29:
		return WvKey::KEY_GRAVE;
	case 0x2A:
		return WvKey::KEY_LEFT_SHIFT;
	case 0x2B:
		return WvKey::KEY_BACKSLASH;
	case 0x2C:
		return WvKey::KEY_Z;
	case 0x2D:
		return WvKey::KEY_X;
	case 0x2E:
		return WvKey::KEY_C;
	case 0x2F:
		return WvKey::KEY_V;
	case 0x30:
		return WvKey::KEY_B;
	case 0x31:
		return WvKey::KEY_N;
	case 0x32:
		return WvKey::KEY_M;
	case 0x33:
		return WvKey::KEY_COMMA;
	case 0x34:
		return WvKey::KEY_PERIOD;
	case 0x35:
		return WvKey::KEY_SLASH;
	case 0x36:
		return WvKey::KEY_RIGHT_SHIFT;
	case 0x37:
		return WvKey::KEY_KP_MULTIPLY;
	case 0x38:
		return WvKey::KEY_LEFT_ALT;
	case 0x39:
		return WvKey::KEY_SPACE;
	case 0x3A:
		return WvKey::KEY_CAPS_LOCK;
	case 0x3B:
		return WvKey::KEY_F1;
	case 0x3C:
		return WvKey::KEY_F2;
	case 0x3D:
		return WvKey::KEY_F3;
	case 0x3E:
		return WvKey::KEY_F4;
	case 0x3F:
		return WvKey::KEY_F5;
	case 0x40:
		return WvKey::KEY_F6;
	case 0x41:
		return WvKey::KEY_F7;
	case 0x42:
		return WvKey::KEY_F8;
	case 0x43:
		return WvKey::KEY_F9;
	case 0x44:
		return WvKey::KEY_F10;
	case 0x45:
		return WvKey::KEY_PAUSE;
	case 0x46:
		return WvKey::KEY_SCROLL_LOCK;
	case 0x47:
		return WvKey::KEY_KP_7;
	case 0x48:
		return WvKey::KEY_KP_8;
	case 0x49:
		return WvKey::KEY_KP_9;
	case 0x4A:
		return WvKey::KEY_KP_MINUS;
	case 0x4B:
		return WvKey::KEY_KP_4;
	case 0x4C:
		return WvKey::KEY_KP_5;
	case 0x4D:
		return WvKey::KEY_KP_6;
	case 0x4E:
		return WvKey::KEY_KP_PLUS;
	case 0x4F:
		return WvKey::KEY_KP_1;
	case 0x50:
		return WvKey::KEY_KP_2;
	case 0x51:
		return WvKey::KEY_KP_3;
	case 0x52:
		return WvKey::KEY_KP_0;
	case 0x53:
		return WvKey::KEY_KP_PERIOD;
	case 0x54:
		return WvKey::KEY_PRINT_SCREEN;
	case 0x56:
		return WvKey::KEY_NON_US_BACKSLASH;
	case 0x57:
		return WvKey::KEY_F11;
	case 0x58:
		return WvKey::KEY_F12;
	default:
		return WvKey::KEY_NONE;
	}
}

WvKey WvInputKeyMap::FromWinExtendedScancode(const uint32_t udwScancode)
{
	switch (udwScancode)
	{
	case 0x1C:
		return WvKey::KEY_KP_ENTER;
	case 0x1D:
		return WvKey::KEY_RIGHT_CTRL;
	case 0x35:
		return WvKey::KEY_KP_DIVIDE;
	case 0x37:
		return WvKey::KEY_PRINT_SCREEN;
	case 0x38:
		return WvKey::KEY_RIGHT_ALT;
	case 0x45:
		return WvKey::KEY_NUM_LOCK;
	case 0x47:
		return WvKey::KEY_HOME;
	case 0x48:
		return WvKey::KEY_UP;
	case 0x49:
		return WvKey::KEY_PAGE_UP;
	case 0x4B:
		return WvKey::KEY_LEFT;
	case 0x4D:
		return WvKey::KEY_RIGHT;
	case 0x4F:
		return WvKey::KEY_END;
	case 0x50:
		return WvKey::KEY_DOWN;
	case 0x51:
		return WvKey::KEY_PAGE_DOWN;
	case 0x52:
		return WvKey::KEY_INSERT;
	case 0x53:
		return WvKey::KEY_DELETE;
	case 0x5B:
		return WvKey::KEY_LEFT_GUI;
	case 0x5C:
		return WvKey::KEY_RIGHT_GUI;
	case 0x5D:
		return WvKey::KEY_APPLICATION;
	default:
		return WvKey::KEY_NONE;
	}
}

WvKey WvInputKeyMap::FromDomCode(const char* const pszCode)
{
	if (pszCode == nullptr || pszCode[0] == '\0')
	{
		return WvKey::KEY_NONE;
	}

	for (uint32_t i = 0; i < s_udwKeyCount; i++)
	{
		if (std::strcmp(s_asKeys[i].m_pszDomCode, pszCode) == 0)
		{
			return s_asKeys[i].m_eKey;
		}
	}

	return WvKey::KEY_NONE;
}

const char* WvInputKeyMap::GetName(const WvKey eKey)
{
	for (uint32_t i = 0; i < s_udwKeyCount; i++)
	{
		if (s_asKeys[i].m_eKey == eKey)
		{
			return s_asKeys[i].m_pszName;
		}
	}

	return "?";
}

} // namespace weev
