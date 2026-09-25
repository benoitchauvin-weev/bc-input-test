#pragma once

// The one window, and the OS message pump that feeds input.

namespace weev
{

class WvWindow
{
public:
	static bool Create(const wchar_t* pwszTitle, int nWidth, int nHeight);
	static void Shutdown();

	// Dispatches every pending OS message. Input arrives through here.
	static void PumpMessages();

	static bool IsQuitRequested();
	static void RequestQuit();

	// The HWND, as a void* so this header needs no windows.h.
	static void* GetNativeHandle();

private:
	static bool s_bQuitRequested;
};

} // namespace weev
