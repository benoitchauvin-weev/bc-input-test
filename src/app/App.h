#pragma once

// The frame loop. Each frame, in order:
//
//   1. pump OS messages          backends Push into the queue
//   2. advance the timer         measured, fixed or replayed step
//   3. poll gamepads             as late as possible, for latency
//   4. drain and build           one immutable snapshot, double buffered
//   5. update the game           gameplay, physics, animation
//   6. record or check           events, step and state hash per frame
//   7. render
//
// Parameters:
//   --test              run the input self-tests and exit
//   --probe_frame_rate  compare jumps across frame rates and exit
//   --frames=N          quit after N frames
//   --fixed_dt[=us]     advance every frame by a constant step (16667 us)
//   --headless          no window; needs --fixed_dt or --replay
//   --agent             a scripted agent plays player 2
//   --record=PATH       record the run
//   --replay=PATH       replay a recording, ignoring live input

#include "game/Game.h"
#include "game/ScriptedAgent.h"
#include "input/InputFrameBuilder.h"
#include "input/InputRecording.h"
#include "timer/Timer.h"

namespace weev
{

class WvApp
{
public:
	static int Run(int nArgCount, char** ppszArgs);

private:
	enum class Mode
	{
		MODE_LIVE,
		MODE_FIXED,
		MODE_REPLAY,
	};

	static const uint32_t APP_MAX_EVENTS_PER_FRAME = 512;

	bool Init();
	void Shutdown();
	int Loop();

	uint64_t AdvanceTimer(uint64_t uqwFrameIndex);
	bool CheckReplay(uint64_t uqwFrameIndex, uint64_t uqwStateHash);

	WvTimer m_oTimer;
	WvGame m_oGame;
	WvInputFrameBuilder m_oInputBuilder;
	WvInputRecording m_oRecording;
	WvScriptedAgent m_oAgent;

	WvInputEvent m_asEvents[APP_MAX_EVENTS_PER_FRAME] = {};

	Mode m_eMode = Mode::MODE_LIVE;
	uint64_t m_uqwFixedStepMicroseconds = 16667;
	uint64_t m_uqwFrameLimit = 0;
	uint64_t m_uqwDivergedFrame = 0;
	const char* m_pszRecordPath = nullptr;
	bool m_bHeadless = false;
	bool m_bAgent = false;
	bool m_bRecording = false;
	bool m_bWindow = false;
};

} // namespace weev
