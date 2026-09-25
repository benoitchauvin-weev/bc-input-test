#pragma once

// --probe_frame_rate: how much the mock game's outcome depends on the
// frame rate, now that the game runs on a variable step.
//
// Plays one input script, timed in wall-clock seconds, at several frame
// rates and with a jittery step, and reports each jump's apex, air time
// and landing point. Events land on the first frame that covers their
// time, as live input would.

namespace weev
{

class WvFrameRateProbe
{
public:
	static void Run();
};

} // namespace weev
