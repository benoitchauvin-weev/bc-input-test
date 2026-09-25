#pragma once

// A GDI stand-in for a renderer: the two players, and a readout of the
// input snapshot the frame read. Presentation only: it reads, never
// writes, game state.

#include "game/World.h"
#include "input/InputFrame.h"

namespace weev
{

struct WvRenderStats
{
	const char* m_pszMode = "live";
	uint32_t m_udwQueueFolded = 0;
	uint32_t m_udwQueueDropped = 0;
	uint64_t m_uqwStateHash = 0;
	bool m_bAgent = false;
};

class WvRenderer
{
public:
	static bool Init(void* pWindow);
	static void Shutdown();

	static void Draw(
		const WvWorld& sWorld,
		const WvInputFrame& oInput,
		const WvRenderStats& sStats
	);
};

} // namespace weev
