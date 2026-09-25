#include "window/Window.h"

#include "input/Input_Win.h"

namespace weev
{

static const wchar_t* const WINDOW_CLASS_NAME = L"BcInputTestWindow";

bool WvWindow::s_bQuitRequested = false;

static HWND s_pWindow = NULL;
static HINSTANCE s_pInstance = NULL;

class WvWindowWin
{
public:
	static LRESULT CALLBACK
	WindowProc(HWND pWindow, UINT udwMessage, WPARAM uqwParam, LPARAM qwParam);
};

LRESULT CALLBACK WvWindowWin::WindowProc(
	const HWND pWindow,
	const UINT udwMessage,
	const WPARAM uqwParam,
	const LPARAM qwParam
)
{
	// Input first: it sees every message, and says which ones it has
	// fully handled.
	LRESULT qwResult = 0;

	if (WvInputWin::HandleMessage(
			pWindow, udwMessage, uqwParam, qwParam, qwResult
		))
	{
		return qwResult;
	}

	switch (udwMessage)
	{
	case WM_CLOSE:
		WvWindow::RequestQuit();
		return 0;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		// The frame loop draws every frame; just mark the area clean.
		ValidateRect(pWindow, NULL);
		return 0;

	case WM_DPICHANGED:
	{
		const RECT* const psSuggested = (const RECT*)qwParam;

		if (psSuggested != NULL)
		{
			SetWindowPos(
				pWindow,
				NULL,
				psSuggested->left,
				psSuggested->top,
				psSuggested->right - psSuggested->left,
				psSuggested->bottom - psSuggested->top,
				SWP_NOZORDER | SWP_NOACTIVATE
			);
		}

		return 0;
	}

	default:
		break;
	}

	return DefWindowProcW(pWindow, udwMessage, uqwParam, qwParam);
}

bool WvWindow::Create(
	const wchar_t* const pwszTitle, const int nWidth, const int nHeight
)
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	s_pInstance = GetModuleHandleW(NULL);

	WNDCLASSEXW sClass = {};
	sClass.cbSize = sizeof(sClass);
	sClass.style = CS_HREDRAW | CS_VREDRAW;
	sClass.lpfnWndProc = &WvWindowWin::WindowProc;
	sClass.hInstance = s_pInstance;
	sClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
	sClass.lpszClassName = WINDOW_CLASS_NAME;

	if (RegisterClassExW(&sClass) == 0)
	{
		return false;
	}

	const DWORD udwStyle = WS_OVERLAPPEDWINDOW;
	const UINT udwDpi = GetDpiForSystem();

	RECT sRect = {};
	sRect.right = nWidth * int(udwDpi) / 96;
	sRect.bottom = nHeight * int(udwDpi) / 96;
	AdjustWindowRectExForDpi(&sRect, udwStyle, FALSE, 0, udwDpi);

	s_pWindow = CreateWindowExW(
		0,
		WINDOW_CLASS_NAME,
		pwszTitle,
		udwStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		sRect.right - sRect.left,
		sRect.bottom - sRect.top,
		NULL,
		NULL,
		s_pInstance,
		NULL
	);

	if (s_pWindow == NULL)
	{
		return false;
	}

	ShowWindow(s_pWindow, SW_SHOW);
	SetForegroundWindow(s_pWindow);
	SetFocus(s_pWindow);

	WvInputWin::AttachWindow(s_pWindow);
	return true;
}

void WvWindow::Shutdown()
{
	WvInputWin::DetachWindow();

	if (s_pWindow != NULL)
	{
		DestroyWindow(s_pWindow);
		s_pWindow = NULL;
	}

	UnregisterClassW(WINDOW_CLASS_NAME, s_pInstance);
	s_bQuitRequested = false;
}

void WvWindow::PumpMessages()
{
	MSG sMessage = {};

	while (PeekMessageW(&sMessage, NULL, 0, 0, PM_REMOVE) != 0)
	{
		if (sMessage.message == WM_QUIT)
		{
			s_bQuitRequested = true;
		}

		TranslateMessage(&sMessage);
		DispatchMessageW(&sMessage);
	}
}

bool WvWindow::IsQuitRequested()
{
	return s_bQuitRequested;
}

void WvWindow::RequestQuit()
{
	s_bQuitRequested = true;
}

void* WvWindow::GetNativeHandle()
{
	return (void*)s_pWindow;
}

} // namespace weev
