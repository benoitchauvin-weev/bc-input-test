#include "render/Renderer.h"

#include "input/InputKeyMap.h"

#define WIN32_LEAN_AND_MEAN (1)
#define NOMINMAX (1)
#include <windows.h>

#include <cstdio>
#include <cstring>

namespace weev
{

static const uint32_t RENDER_EDGE_LOG_LINES = 10;
static const uint32_t RENDER_LINE_CHARS = 160;
static const float RENDER_WORLD_WIDTH = 22.0f;

static HWND s_pWindow = NULL;
static HDC s_pBackDc = NULL;
static HBITMAP s_pBackBitmap = NULL;
static HGDIOBJ s_pOldBitmap = NULL;
static HFONT s_pFont = NULL;
static int s_nBackWidth = 0;
static int s_nBackHeight = 0;

// The last few frames that had presses or releases, newest first, so a
// one-frame event stays readable.
static char s_aaszEdgeLog[RENDER_EDGE_LOG_LINES][RENDER_LINE_CHARS] = {};

class WvRendererWin
{
public:
	static void EnsureBackBuffer(HDC pWindowDc, int nWidth, int nHeight);
	static void LogEdges(const WvInputFrame& oInput);
	static void DrawPlayer(
		const WvWorld& sWorld,
		uint32_t udwPlayer,
		int nWidth,
		int nFloorY,
		COLORREF udwColour
	);
	static void Text(int nX, int nY, COLORREF udwColour, const char* pszText);
};

void WvRendererWin::EnsureBackBuffer(
	const HDC pWindowDc, const int nWidth, const int nHeight
)
{
	if (s_pBackDc != NULL && nWidth == s_nBackWidth && nHeight == s_nBackHeight)
	{
		return;
	}

	if (s_pBackDc != NULL)
	{
		SelectObject(s_pBackDc, s_pOldBitmap);
		DeleteObject(s_pBackBitmap);
		DeleteDC(s_pBackDc);
	}

	s_pBackDc = CreateCompatibleDC(pWindowDc);
	s_pBackBitmap = CreateCompatibleBitmap(pWindowDc, nWidth, nHeight);
	s_pOldBitmap = SelectObject(s_pBackDc, s_pBackBitmap);
	s_nBackWidth = nWidth;
	s_nBackHeight = nHeight;
}

void WvRendererWin::Text(
	const int nX, const int nY, const COLORREF udwColour, const char* pszText
)
{
	SetTextColor(s_pBackDc, udwColour);
	TextOutA(s_pBackDc, nX, nY, pszText, int(std::strlen(pszText)));
}

void WvRendererWin::LogEdges(const WvInputFrame& oInput)
{
	char szLine[RENDER_LINE_CHARS] = {};
	int nLength = std::snprintf(
		szLine,
		sizeof(szLine),
		"frame %-6llu",
		(unsigned long long)oInput.GetFrameIndex()
	);
	bool bAny = false;

	for (uint32_t i = 0; i < oInput.GetKeyEdgeCount(); i++)
	{
		const WvInputKeyEdge& sEdge = oInput.GetKeyEdge(i);

		if (nLength >= int(sizeof(szLine)) - 1)
		{
			break;
		}

		nLength += std::snprintf(
			szLine + nLength,
			sizeof(szLine) - size_t(nLength),
			"  %s down x%u up x%u",
			WvInputKeyMap::GetName(WvKey(sEdge.m_byKey)),
			(unsigned)sEdge.m_byPressCount,
			(unsigned)sEdge.m_byReleaseCount
		);
		bAny = true;
	}

	for (uint32_t udwSlot = 0; udwSlot < INPUT_GAMEPAD_SLOT_COUNT; udwSlot++)
	{
		for (uint32_t b = 0; b < uint32_t(WvGamepadButton::BUTTON_COUNT); b++)
		{
			const uint32_t udwDown =
				oInput.GetGamepadPressCount(udwSlot, WvGamepadButton(b));
			const uint32_t udwUp =
				oInput.GetGamepadReleaseCount(udwSlot, WvGamepadButton(b));

			if ((udwDown == 0 && udwUp == 0) ||
				nLength >= int(sizeof(szLine)) - 1)
			{
				continue;
			}

			nLength += std::snprintf(
				szLine + nLength,
				sizeof(szLine) - size_t(nLength),
				"  pad%u.b%u down x%u up x%u",
				udwSlot,
				b,
				udwDown,
				udwUp
			);
			bAny = true;
		}
	}

	if (oInput.IsFocusLostThisFrame() && nLength < int(sizeof(szLine)) - 1)
	{
		std::snprintf(
			szLine + nLength,
			sizeof(szLine) - size_t(nLength),
			"  FOCUS LOST: human input released"
		);
		bAny = true;
	}

	if (!bAny)
	{
		return;
	}

	for (uint32_t i = RENDER_EDGE_LOG_LINES - 1; i > 0; i--)
	{
		std::memcpy(s_aaszEdgeLog[i], s_aaszEdgeLog[i - 1], RENDER_LINE_CHARS);
	}

	std::memcpy(s_aaszEdgeLog[0], szLine, RENDER_LINE_CHARS);
}

void WvRendererWin::DrawPlayer(
	const WvWorld& sWorld,
	const uint32_t udwPlayer,
	const int nWidth,
	const int nFloorY,
	const COLORREF udwColour
)
{
	const float fPixelsPerMetre = float(nWidth) / RENDER_WORLD_WIDTH;
	const float fCentreX = float(nWidth) * 0.5f +
						   sWorld.m_afPositionX[udwPlayer] * fPixelsPerMetre;
	const float fBottomY = float(nFloorY) -
						   sWorld.m_afPositionY[udwPlayer] * fPixelsPerMetre;
	const float fHalfWidth = 0.4f * sWorld.m_afScaleX[udwPlayer] *
							 fPixelsPerMetre;
	const float fHeight = 1.2f * sWorld.m_afScaleY[udwPlayer] * fPixelsPerMetre;

	RECT sBody = {};
	sBody.left = LONG(fCentreX - fHalfWidth);
	sBody.right = LONG(fCentreX + fHalfWidth);
	sBody.top = LONG(fBottomY - fHeight);
	sBody.bottom = LONG(fBottomY);

	const HBRUSH pBrush = CreateSolidBrush(udwColour);
	FillRect(s_pBackDc, &sBody, pBrush);
	DeleteObject(pBrush);

	static const char* const s_apszStates[] = {
		"idle", "run", "jump", "fall", "land"
	};
	char szLabel[64] = {};
	std::snprintf(
		szLabel,
		sizeof(szLabel),
		"P%u %s  jumps %u",
		udwPlayer + 1,
		s_apszStates[uint32_t(sWorld.m_aeAnimState[udwPlayer])],
		sWorld.m_udwJumpCount[udwPlayer]
	);
	Text(int(fCentreX) - 50, int(sBody.top) - 22, udwColour, szLabel);
}

bool WvRenderer::Init(void* const pWindow)
{
	s_pWindow = (HWND)pWindow;
	s_pFont = CreateFontW(
		-15,
		0,
		0,
		0,
		FW_NORMAL,
		FALSE,
		FALSE,
		FALSE,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY,
		FIXED_PITCH | FF_MODERN,
		L"Consolas"
	);
	std::memset(s_aaszEdgeLog, 0, sizeof(s_aaszEdgeLog));
	return s_pWindow != NULL;
}

void WvRenderer::Shutdown()
{
	if (s_pBackDc != NULL)
	{
		SelectObject(s_pBackDc, s_pOldBitmap);
		DeleteObject(s_pBackBitmap);
		DeleteDC(s_pBackDc);
		s_pBackDc = NULL;
	}

	if (s_pFont != NULL)
	{
		DeleteObject(s_pFont);
		s_pFont = NULL;
	}

	s_pWindow = NULL;
}

void WvRenderer::Draw(
	const WvWorld& sWorld,
	const WvInputFrame& oInput,
	const WvRenderStats& sStats
)
{
	if (s_pWindow == NULL)
	{
		return;
	}

	WvRendererWin::LogEdges(oInput);

	RECT sClient = {};
	GetClientRect(s_pWindow, &sClient);
	const int nWidth = sClient.right > 0 ? sClient.right : 1;
	const int nHeight = sClient.bottom > 0 ? sClient.bottom : 1;

	const HDC pWindowDc = GetDC(s_pWindow);
	WvRendererWin::EnsureBackBuffer(pWindowDc, nWidth, nHeight);

	const HBRUSH pGround = CreateSolidBrush(RGB(28, 29, 58));
	FillRect(s_pBackDc, &sClient, pGround);
	DeleteObject(pGround);

	const int nFloorY = nHeight * 58 / 100;
	RECT sFloor = {0, nFloorY, nWidth, nFloorY + 3};
	const HBRUSH pFloor = CreateSolidBrush(RGB(120, 124, 170));
	FillRect(s_pBackDc, &sFloor, pFloor);
	DeleteObject(pFloor);

	SetBkMode(s_pBackDc, TRANSPARENT);
	const HGDIOBJ pOldFont = SelectObject(s_pBackDc, s_pFont);

	WvRendererWin::DrawPlayer(sWorld, 0, nWidth, nFloorY, RGB(255, 200, 87));
	WvRendererWin::DrawPlayer(sWorld, 1, nWidth, nFloorY, RGB(63, 196, 206));

	const COLORREF udwInk = RGB(230, 230, 245);
	const COLORREF udwSoft = RGB(160, 163, 200);
	char szLine[RENDER_LINE_CHARS] = {};
	int nY = 10;
	const int nLine = 18;

	static const char* const s_apszActivity[] = {
		"focused", "unfocused", "overlay", "suspended"
	};

	std::snprintf(
		szLine,
		sizeof(szLine),
		"frame %llu   step %.2f ms   mode %s%s   %s   hash %016llx",
		(unsigned long long)oInput.GetFrameIndex(),
		double(oInput.GetStepMicroseconds()) / 1000.0,
		sStats.m_pszMode,
		sStats.m_bAgent ? " + agent (P2)" : "",
		s_apszActivity[uint32_t(oInput.GetActivity())],
		(unsigned long long)sStats.m_uqwStateHash
	);
	WvRendererWin::Text(10, nY, udwInk, szLine);
	nY += nLine;

	int nLength = std::snprintf(szLine, sizeof(szLine), "keys down:");

	for (uint32_t k = 0; k < INPUT_KEY_COUNT; k++)
	{
		if (oInput.IsDown(WvKey(k)) && nLength < int(sizeof(szLine)) - 16)
		{
			nLength += std::snprintf(
				szLine + nLength,
				sizeof(szLine) - size_t(nLength),
				" %s",
				WvInputKeyMap::GetName(WvKey(k))
			);
		}
	}

	WvRendererWin::Text(10, nY, udwInk, szLine);
	nY += nLine;

	int32_t dwMouseX = 0;
	int32_t dwMouseY = 0;
	int32_t dwDeltaX = 0;
	int32_t dwDeltaY = 0;
	oInput.GetMousePosition(dwMouseX, dwMouseY);
	oInput.GetMouseDelta(dwDeltaX, dwDeltaY);
	int32_t dwViewportWidth = 0;
	int32_t dwViewportHeight = 0;
	oInput.GetViewport(dwViewportWidth, dwViewportHeight);

	std::snprintf(
		szLine,
		sizeof(szLine),
		"mouse %.1f,%.1f px   raw delta %d,%d   wheel %d   buttons %c%c%c   "
		"viewport %dx%d",
		double(dwMouseX) / INPUT_POSITION_SCALE,
		double(dwMouseY) / INPUT_POSITION_SCALE,
		dwDeltaX,
		dwDeltaY,
		oInput.GetWheel(),
		oInput.IsMouseDown(WvMouseButton::BUTTON_LEFT) ? 'L' : '-',
		oInput.IsMouseDown(WvMouseButton::BUTTON_MIDDLE) ? 'M' : '-',
		oInput.IsMouseDown(WvMouseButton::BUTTON_RIGHT) ? 'R' : '-',
		dwViewportWidth,
		dwViewportHeight
	);
	WvRendererWin::Text(10, nY, udwInk, szLine);
	nY += nLine;

	for (uint32_t udwSlot = 0; udwSlot < INPUT_GAMEPAD_SLOT_COUNT; udwSlot++)
	{
		const WvInputPadState& sPad = oInput.GetGamepad(udwSlot);

		if (sPad.m_udwGeneration == 0)
		{
			continue;
		}

		std::snprintf(
			szLine,
			sizeof(szLine),
			"pad slot %u  %s  id %08x  gen %u  LX %6d LY %6d  LT %5d RT %5d  "
			"buttons %04x",
			udwSlot,
			sPad.m_byConnected != 0 ? "connected   " : "disconnected",
			sPad.m_udwStableId,
			sPad.m_udwGeneration,
			sPad.m_awAxes[uint32_t(WvGamepadAxis::AXIS_LEFT_X)],
			sPad.m_awAxes[uint32_t(WvGamepadAxis::AXIS_LEFT_Y)],
			sPad.m_awAxes[uint32_t(WvGamepadAxis::AXIS_LEFT_TRIGGER)],
			sPad.m_awAxes[uint32_t(WvGamepadAxis::AXIS_RIGHT_TRIGGER)],
			(unsigned)sPad.m_uwButtonsDown
		);
		WvRendererWin::Text(10, nY, udwInk, szLine);
		nY += nLine;
	}

	std::snprintf(
		szLine,
		sizeof(szLine),
		"queue: folded %u   dropped %u",
		sStats.m_udwQueueFolded,
		sStats.m_udwQueueDropped
	);
	WvRendererWin::Text(10, nY, udwSoft, szLine);

	int nLogY = nFloorY + 20;
	WvRendererWin::Text(
		10, nLogY, udwSoft, "recent frames with presses or releases:"
	);
	nLogY += nLine;

	for (uint32_t i = 0; i < RENDER_EDGE_LOG_LINES; i++)
	{
		WvRendererWin::Text(10, nLogY, udwInk, s_aaszEdgeLog[i]);
		nLogY += nLine;
	}

	WvRendererWin::Text(
		10,
		nHeight - 24,
		udwSoft,
		"P1: A/D move, Space or W jump, pad slot 0   P2: arrows, Up or RCtrl "
		"jump, pad slot 1   R reset   Esc quit"
	);

	SelectObject(s_pBackDc, pOldFont);
	BitBlt(pWindowDc, 0, 0, nWidth, nHeight, s_pBackDc, 0, 0, SRCCOPY);
	ReleaseDC(s_pWindow, pWindowDc);
}

} // namespace weev
