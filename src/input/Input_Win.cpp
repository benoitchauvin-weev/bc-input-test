#include "input/Input.h"
#include "input/InputKeyMap.h"
#include "input/Input_Win.h"

#include "timer/Timer.h"

#include <windowsx.h>
#include <Xinput.h>

namespace weev
{

// Every logical coordinate on Windows is expressed against this DPI.
static const int INPUT_WIN_BASE_DPI = 96;

// XInput exposes four users. Polling an empty one is slow, so empty
// users are probed about once a second instead of every frame.
static const uint32_t INPUT_WIN_XINPUT_USERS = 4;
static const uint64_t INPUT_WIN_PROBE_INTERVAL_MICROSECONDS = 1000000;
static const uint8_t INPUT_WIN_NO_SLOT = 0xFF;

typedef DWORD(WINAPI* XInputGetStateFunction)(DWORD, XINPUT_STATE*);

struct WvInputWinPadUser
{
	bool m_bConnected;
	uint8_t m_bySlot;
	DWORD m_udwLastPacket;
	WORD m_uwButtons;
	int16_t m_awAxes[uint32_t(WvGamepadAxis::AXIS_COUNT)];
	uint64_t m_uqwNextProbeTicks;
};

static HWND s_pWindow = NULL;
static HMODULE s_pXInputModule = NULL;
static XInputGetStateFunction s_pfnXInputGetState = NULL;
static WvInputWinPadUser s_asPadUsers[INPUT_WIN_XINPUT_USERS] = {};

// Which XInput user last owned each slot. A slot is a device, never a
// player: it is not handed to another device while its owner may come
// back.
static uint8_t s_abySlotOwner[INPUT_GAMEPAD_SLOT_COUNT] = {};

// Mouse buttons this backend has seen go down, for capture.
static uint8_t s_byMouseHeld = 0;

// XInput buttons in WvGamepadButton order.
static const WORD s_auwXInputButtons[] = {
	XINPUT_GAMEPAD_A,
	XINPUT_GAMEPAD_B,
	XINPUT_GAMEPAD_X,
	XINPUT_GAMEPAD_Y,
	XINPUT_GAMEPAD_LEFT_SHOULDER,
	XINPUT_GAMEPAD_RIGHT_SHOULDER,
	XINPUT_GAMEPAD_BACK,
	XINPUT_GAMEPAD_START,
	XINPUT_GAMEPAD_LEFT_THUMB,
	XINPUT_GAMEPAD_RIGHT_THUMB,
	XINPUT_GAMEPAD_DPAD_UP,
	XINPUT_GAMEPAD_DPAD_DOWN,
	XINPUT_GAMEPAD_DPAD_LEFT,
	XINPUT_GAMEPAD_DPAD_RIGHT,
};

class WvInputWinPads
{
public:
	static void Load();
	static void Unload();
	static void Poll();

private:
	static void PollUser(uint32_t udwUser, uint64_t uqwNow);
	static uint8_t ClaimSlot(uint32_t udwUser);
	static void PushState(WvInputWinPadUser& sUser, const XINPUT_GAMEPAD& sPad);
	static uint64_t MicrosecondsToTicks(uint64_t uqwMicroseconds);
};

void WvInputWinPads::Load()
{
	for (uint32_t i = 0; i < INPUT_GAMEPAD_SLOT_COUNT; i++)
	{
		s_abySlotOwner[i] = INPUT_WIN_NO_SLOT;
	}

	for (uint32_t i = 0; i < INPUT_WIN_XINPUT_USERS; i++)
	{
		s_asPadUsers[i] = {};
		s_asPadUsers[i].m_bySlot = INPUT_WIN_NO_SLOT;
	}

	// Loaded at run time so nothing links against XInput: a system
	// component on every supported Windows, but absent is not fatal.
	s_pXInputModule = LoadLibraryW(L"xinput1_4.dll");

	if (s_pXInputModule == NULL)
	{
		s_pXInputModule = LoadLibraryW(L"xinput9_1_0.dll");
	}

	if (s_pXInputModule != NULL)
	{
		s_pfnXInputGetState = (XInputGetStateFunction)(void*)
			GetProcAddress(s_pXInputModule, "XInputGetState");
	}
}

void WvInputWinPads::Unload()
{
	s_pfnXInputGetState = NULL;

	if (s_pXInputModule != NULL)
	{
		FreeLibrary(s_pXInputModule);
		s_pXInputModule = NULL;
	}
}

uint64_t WvInputWinPads::MicrosecondsToTicks(const uint64_t uqwMicroseconds)
{
	return uqwMicroseconds * WvTimer::GetTicksPerSecond() / 1000000;
}

void WvInputWinPads::Poll()
{
	if (s_pfnXInputGetState == NULL)
	{
		return;
	}

	const uint64_t uqwNow = WvTimer::GetTicks();

	for (uint32_t i = 0; i < INPUT_WIN_XINPUT_USERS; i++)
	{
		PollUser(i, uqwNow);
	}
}

uint8_t WvInputWinPads::ClaimSlot(const uint32_t udwUser)
{
	WvInputWinPadUser& sUser = s_asPadUsers[udwUser];

	// The same device comes back to the same slot.
	if (sUser.m_bySlot != INPUT_WIN_NO_SLOT)
	{
		return sUser.m_bySlot;
	}

	for (uint32_t i = 0; i < INPUT_GAMEPAD_SLOT_COUNT; i++)
	{
		if (s_abySlotOwner[i] == INPUT_WIN_NO_SLOT)
		{
			s_abySlotOwner[i] = uint8_t(udwUser);
			sUser.m_bySlot = uint8_t(i);
			return sUser.m_bySlot;
		}
	}

	return INPUT_WIN_NO_SLOT;
}

void WvInputWinPads::PollUser(const uint32_t udwUser, const uint64_t uqwNow)
{
	WvInputWinPadUser& sUser = s_asPadUsers[udwUser];

	if (!sUser.m_bConnected && uqwNow < sUser.m_uqwNextProbeTicks)
	{
		return;
	}

	XINPUT_STATE sState = {};
	const DWORD udwResult = s_pfnXInputGetState(udwUser, &sState);

	if (udwResult != ERROR_SUCCESS)
	{
		if (sUser.m_bConnected)
		{
			WvInput::Push(WvInputEvent::MakeDisconnect(sUser.m_bySlot));
			sUser.m_bConnected = false;
			sUser.m_uwButtons = 0;

			for (uint32_t i = 0; i < uint32_t(WvGamepadAxis::AXIS_COUNT); i++)
			{
				sUser.m_awAxes[i] = 0;
			}
		}

		sUser.m_uqwNextProbeTicks =
			uqwNow + MicrosecondsToTicks(INPUT_WIN_PROBE_INTERVAL_MICROSECONDS);
		return;
	}

	if (!sUser.m_bConnected)
	{
		const uint8_t bySlot = ClaimSlot(udwUser);

		if (bySlot == INPUT_WIN_NO_SLOT)
		{
			return;
		}

		// "XI" and the user index: stable for as long as XInput keeps
		// the pad on the same user.
		const uint32_t udwStableId = 0x58490000u | udwUser;

		WvInput::Push(
			WvInputEvent::MakeConnect(
				bySlot, udwStableId, WvGamepadFamily::FAMILY_XBOX, 0
			)
		);
		sUser.m_bConnected = true;
		sUser.m_udwLastPacket = sState.dwPacketNumber - 1;
	}

	if (sState.dwPacketNumber == sUser.m_udwLastPacket)
	{
		return;
	}

	sUser.m_udwLastPacket = sState.dwPacketNumber;
	PushState(sUser, sState.Gamepad);
}

void WvInputWinPads::PushState(
	WvInputWinPadUser& sUser, const XINPUT_GAMEPAD& sPad
)
{
	const uint32_t udwButtonCount = sizeof(s_auwXInputButtons) /
									sizeof(s_auwXInputButtons[0]);

	for (uint32_t i = 0; i < udwButtonCount; i++)
	{
		const WORD uwMask = s_auwXInputButtons[i];
		const bool bWas = (sUser.m_uwButtons & uwMask) != 0;
		const bool bIs = (sPad.wButtons & uwMask) != 0;

		if (bWas != bIs)
		{
			WvInput::Push(
				WvInputEvent::MakePadButton(
					sUser.m_bySlot,
					WvGamepadButton(i),
					bIs,
					WvInputSource::SOURCE_HUMAN
				)
			);
		}
	}

	sUser.m_uwButtons = sPad.wButtons;

	// Triggers are 0..255; widened to the shared 0..32767 range.
	const int16_t awAxes[uint32_t(WvGamepadAxis::AXIS_COUNT)] = {
		sPad.sThumbLX,
		sPad.sThumbLY,
		sPad.sThumbRX,
		sPad.sThumbRY,
		int16_t(sPad.bLeftTrigger * 32767 / 255),
		int16_t(sPad.bRightTrigger * 32767 / 255),
	};

	for (uint32_t i = 0; i < uint32_t(WvGamepadAxis::AXIS_COUNT); i++)
	{
		if (awAxes[i] == sUser.m_awAxes[i])
		{
			continue;
		}

		sUser.m_awAxes[i] = awAxes[i];
		WvInput::Push(
			WvInputEvent::MakePadAxis(
				sUser.m_bySlot,
				WvGamepadAxis(i),
				awAxes[i],
				WvInputSource::SOURCE_HUMAN
			)
		);
	}
}

bool WvInput::PlatformInit()
{
	WvInputWinPads::Load();
	s_byMouseHeld = 0;
	return true;
}

void WvInput::PlatformShutdown()
{
	WvInputWin::DetachWindow();
	WvInputWinPads::Unload();
}

void WvInput::PlatformPoll()
{
	WvInputWinPads::Poll();
}

uint32_t WvInput::PlatformGetThreadId()
{
	return uint32_t(GetCurrentThreadId());
}

void WvInputWin::AttachWindow(const HWND pWindow)
{
	s_pWindow = pWindow;

	// Raw input for mouse deltas, which WM_MOUSEMOVE cannot give once
	// the cursor stops at a screen edge. Without RIDEV_NOLEGACY, so the
	// usual mouse messages still arrive for position and buttons.
	RAWINPUTDEVICE sDevice = {};
	sDevice.usUsagePage = 0x01; // generic desktop
	sDevice.usUsage = 0x02;		// mouse
	sDevice.hwndTarget = pWindow;
	RegisterRawInputDevices(&sDevice, 1, sizeof(sDevice));

	const bool bFocused = GetFocus() == pWindow;
	WvInput::Push(
		WvInputEvent::MakeActivity(
			bFocused ? WvInputActivity::ACTIVITY_FOCUSED
					 : WvInputActivity::ACTIVITY_UNFOCUSED
		)
	);
	PushViewport(pWindow);
}

void WvInputWin::DetachWindow()
{
	s_pWindow = NULL;
}

int32_t WvInputWin::ToLogicalPosition(const int nPixels, const UINT udwDpi)
{
	// Same truncation as WvWindow::GetSize in the engine.
	const int nDpi = udwDpi != 0 ? int(udwDpi) : INPUT_WIN_BASE_DPI;
	return int32_t(nPixels * INPUT_POSITION_SCALE * INPUT_WIN_BASE_DPI / nDpi);
}

void WvInputWin::PushViewport(const HWND pWindow)
{
	RECT sClient = {};
	GetClientRect(pWindow, &sClient);

	const UINT udwDpi = GetDpiForWindow(pWindow);
	const int nDpi = udwDpi != 0 ? int(udwDpi) : INPUT_WIN_BASE_DPI;

	WvInput::Push(
		WvInputEvent::MakeViewport(
			(sClient.right - sClient.left) * INPUT_WIN_BASE_DPI / nDpi,
			(sClient.bottom - sClient.top) * INPUT_WIN_BASE_DPI / nDpi,
			nDpi * 100 / INPUT_WIN_BASE_DPI
		)
	);
}

void WvInputWin::ReleaseMouseButtons()
{
	for (uint32_t i = 0; i < INPUT_MOUSE_BUTTON_COUNT; i++)
	{
		if ((s_byMouseHeld & (1u << i)) != 0)
		{
			WvInput::Push(
				WvInputEvent::MakeMouseButton(
					WvMouseButton(i), false, WvInputSource::SOURCE_HUMAN
				)
			);
		}
	}

	s_byMouseHeld = 0;
}

void WvInputWin::HandleMouseButton(const uint32_t udwButton, const bool bDown)
{
	const uint8_t byBit = uint8_t(1u << udwButton);

	if (bDown)
	{
		// Capture, or a button released outside the window is never
		// seen and the button sticks.
		if (s_byMouseHeld == 0 && s_pWindow != NULL)
		{
			SetCapture(s_pWindow);
		}

		s_byMouseHeld |= byBit;
	}
	else
	{
		s_byMouseHeld &= uint8_t(~byBit);
	}

	WvInput::Push(
		WvInputEvent::MakeMouseButton(
			WvMouseButton(udwButton), bDown, WvInputSource::SOURCE_HUMAN
		)
	);

	if (!bDown && s_byMouseHeld == 0)
	{
		ReleaseCapture();
	}
}

void WvInputWin::HandleRawInput(const LPARAM qwParam)
{
	RAWINPUT sRaw = {};
	UINT udwSize = sizeof(sRaw);

	if (GetRawInputData(
			(HRAWINPUT)qwParam,
			RID_INPUT,
			&sRaw,
			&udwSize,
			sizeof(RAWINPUTHEADER)
		) == (UINT)-1)
	{
		return;
	}

	if (sRaw.header.dwType != RIM_TYPEMOUSE)
	{
		return;
	}

	// Absolute mode comes from remote desktop, VMs and tablets. Its
	// values are positions, not deltas; position already comes from
	// WM_MOUSEMOVE, so it is left out here.
	if ((sRaw.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) != 0)
	{
		return;
	}

	if (sRaw.data.mouse.lLastX != 0 || sRaw.data.mouse.lLastY != 0)
	{
		WvInput::Push(
			WvInputEvent::MakeMouseMotion(
				WvMouseMotion::MOTION_MOVE,
				sRaw.data.mouse.lLastX,
				sRaw.data.mouse.lLastY,
				WvInputSource::SOURCE_HUMAN
			)
		);
	}
}

void WvInputWin::HandleKey(
	const HWND pWindow,
	const UINT udwMessage,
	const WPARAM uqwParam,
	const LPARAM qwParam
)
{
	const bool bDown = udwMessage == WM_KEYDOWN || udwMessage == WM_SYSKEYDOWN;
	const bool bWasDown = (qwParam & (1 << 30)) != 0;

	// Auto-repeat: a key-down for a key already down.
	if (bDown && bWasDown)
	{
		return;
	}

	const uint32_t udwVirtualKey = uint32_t(uqwParam);
	uint32_t udwScancode = uint32_t((qwParam >> 16) & 0xFF);
	bool bExtended = ((qwParam >> 24) & 1) != 0;

	// Injected and remapped keys can arrive with no scancode.
	if (udwScancode == 0)
	{
		const UINT udwMapped =
			MapVirtualKeyW(udwVirtualKey, MAPVK_VK_TO_VSC_EX);
		udwScancode = udwMapped & 0xFF;
		bExtended = (udwMapped & 0xFF00) == 0xE000;
	}

	// AltGr sends a fake left Ctrl just before right Alt, with the same
	// message time. Drop the Ctrl.
	if (bDown && udwVirtualKey == VK_CONTROL && !bExtended)
	{
		MSG sNext = {};

		if (PeekMessageW(&sNext, pWindow, 0, 0, PM_NOREMOVE) != 0 &&
			(sNext.message == WM_KEYDOWN || sNext.message == WM_SYSKEYDOWN) &&
			sNext.wParam == VK_MENU && ((sNext.lParam >> 24) & 1) != 0 &&
			sNext.time == (DWORD)GetMessageTime())
		{
			return;
		}
	}

	const WvKey eKey =
		WvInputKeyMap::FromWinScancode(udwScancode, bExtended, udwVirtualKey);

	if (eKey == WvKey::KEY_NONE)
	{
		return;
	}

	// PrintScreen only ever sends a key-up.
	if (eKey == WvKey::KEY_PRINT_SCREEN)
	{
		if (!bDown)
		{
			WvInput::Push(
				WvInputEvent::MakeKey(
					eKey, true, WvInputSource::SOURCE_HUMAN, 0
				)
			);
			WvInput::Push(
				WvInputEvent::MakeKey(
					eKey, false, WvInputSource::SOURCE_HUMAN, 0
				)
			);
		}

		return;
	}

	// With both Shifts held, releasing one sends no key-up for it, so
	// either Shift going up releases both.
	if (!bDown &&
		(eKey == WvKey::KEY_LEFT_SHIFT || eKey == WvKey::KEY_RIGHT_SHIFT))
	{
		WvInput::Push(
			WvInputEvent::MakeKey(
				WvKey::KEY_LEFT_SHIFT, false, WvInputSource::SOURCE_HUMAN, 0
			)
		);
		WvInput::Push(
			WvInputEvent::MakeKey(
				WvKey::KEY_RIGHT_SHIFT, false, WvInputSource::SOURCE_HUMAN, 0
			)
		);
		return;
	}

	WvInput::Push(
		WvInputEvent::MakeKey(eKey, bDown, WvInputSource::SOURCE_HUMAN, 0)
	);
}

bool WvInputWin::HandleMessage(
	const HWND pWindow,
	const UINT udwMessage,
	const WPARAM uqwParam,
	const LPARAM qwParam,
	LRESULT& qwOutResult
)
{
	qwOutResult = 0;

	switch (udwMessage)
	{
	case WM_KEYDOWN:
	case WM_KEYUP:
		HandleKey(pWindow, udwMessage, uqwParam, qwParam);
		return true;

	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		// Alt combinations and F10 arrive here. They still go on to
		// DefWindowProc, so Alt+F4 closes the window.
		HandleKey(pWindow, udwMessage, uqwParam, qwParam);
		return false;

	case WM_SYSCOMMAND:
		// A lone Alt or F10 would enter the modal menu loop and stall
		// the frame loop until the next key.
		if ((uqwParam & 0xFFF0) == SC_KEYMENU)
		{
			return true;
		}
		return false;

	case WM_MOUSEMOVE:
	{
		const UINT udwDpi = GetDpiForWindow(pWindow);

		// Signed: under capture the cursor can be left of or above the
		// client area, and LOWORD would read that as a large number.
		WvInput::Push(
			WvInputEvent::MakeMousePosition(
				ToLogicalPosition(GET_X_LPARAM(qwParam), udwDpi),
				ToLogicalPosition(GET_Y_LPARAM(qwParam), udwDpi),
				WvInputSource::SOURCE_HUMAN
			)
		);
		return true;
	}

	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		HandleMouseButton(
			uint32_t(WvMouseButton::BUTTON_LEFT), udwMessage == WM_LBUTTONDOWN
		);
		return true;

	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
		HandleMouseButton(
			uint32_t(WvMouseButton::BUTTON_RIGHT), udwMessage == WM_RBUTTONDOWN
		);
		return true;

	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
		HandleMouseButton(
			uint32_t(WvMouseButton::BUTTON_MIDDLE), udwMessage == WM_MBUTTONDOWN
		);
		return true;

	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	{
		const WvMouseButton eButton = GET_XBUTTON_WPARAM(uqwParam) == XBUTTON1
										? WvMouseButton::BUTTON_X1
										: WvMouseButton::BUTTON_X2;
		HandleMouseButton(uint32_t(eButton), udwMessage == WM_XBUTTONDOWN);
		qwOutResult = TRUE;
		return true;
	}

	case WM_CAPTURECHANGED:
		// Capture taken by someone else: the button-ups will not come.
		if ((HWND)qwParam != pWindow)
		{
			ReleaseMouseButtons();
		}
		return true;

	case WM_MOUSEWHEEL:
		// Summed, not divided: high-resolution wheels send less than a
		// notch at a time.
		WvInput::Push(
			WvInputEvent::MakeMouseMotion(
				WvMouseMotion::MOTION_WHEEL,
				GET_WHEEL_DELTA_WPARAM(uqwParam),
				0,
				WvInputSource::SOURCE_HUMAN
			)
		);
		return true;

	case WM_MOUSEHWHEEL:
		WvInput::Push(
			WvInputEvent::MakeMouseMotion(
				WvMouseMotion::MOTION_WHEEL_X,
				GET_WHEEL_DELTA_WPARAM(uqwParam),
				0,
				WvInputSource::SOURCE_HUMAN
			)
		);
		return true;

	case WM_INPUT:
		HandleRawInput(qwParam);
		return false;

	case WM_SETFOCUS:
		WvInput::Push(
			WvInputEvent::MakeActivity(WvInputActivity::ACTIVITY_FOCUSED)
		);
		return false;

	case WM_KILLFOCUS:
		ReleaseMouseButtons();
		WvInput::Push(WvInputEvent::MakeFocusLost(WvInputSource::SOURCE_HUMAN));
		WvInput::Push(
			WvInputEvent::MakeActivity(WvInputActivity::ACTIVITY_UNFOCUSED)
		);
		return false;

	case WM_ACTIVATEAPP:
		if (uqwParam == FALSE)
		{
			ReleaseMouseButtons();
			WvInput::Push(
				WvInputEvent::MakeFocusLost(WvInputSource::SOURCE_HUMAN)
			);
		}
		return false;

	case WM_SIZE:
		if (uqwParam == SIZE_MINIMIZED)
		{
			WvInput::Push(
				WvInputEvent::MakeFocusLost(WvInputSource::SOURCE_HUMAN)
			);
			WvInput::Push(
				WvInputEvent::MakeActivity(WvInputActivity::ACTIVITY_SUSPENDED)
			);
		}
		else
		{
			PushViewport(pWindow);
		}
		return false;

	default:
		return false;
	}
}

} // namespace weev
