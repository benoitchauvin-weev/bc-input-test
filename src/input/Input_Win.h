#pragma once

// The Windows backend's entry points from the window procedure. Kept out
// of Input.h so portable code never sees a Windows type.

#define WIN32_LEAN_AND_MEAN (1)
#define NOMINMAX (1)
#include <windows.h>

#include <cstdint>

namespace weev
{

class WvInputWin
{
public:
	// Registers raw mouse input and pushes the window's current focus
	// and viewport, so the first frame starts right.
	static void AttachWindow(HWND pWindow);
	static void DetachWindow();

	// Called first by the window procedure. True when the message is
	// fully handled and must not reach DefWindowProc.
	static bool HandleMessage(
		HWND pWindow,
		UINT udwMessage,
		WPARAM uqwParam,
		LPARAM qwParam,
		LRESULT& qwOutResult
	);

private:
	static void HandleKey(
		HWND pWindow, UINT udwMessage, WPARAM uqwParam, LPARAM qwParam
	);
	static void HandleMouseButton(uint32_t udwButton, bool bDown);
	static void HandleRawInput(LPARAM qwParam);
	static void PushViewport(HWND pWindow);
	static void ReleaseMouseButtons();
	static int32_t ToLogicalPosition(int nPixels, UINT udwDpi);
};

} // namespace weev
