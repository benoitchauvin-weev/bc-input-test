#pragma once

// A recording: for each frame, the events it drained and the step it
// advanced by, plus the game's state hash after it ran. Replaying feeds
// the same events and steps back, so the game must advance by the step
// it is given and never read the clock itself.
//
// Timestamps are left out: a frame is its index, events inside it are
// in arrival order, and nothing else is needed to rebuild it.
//
// Input-only determinism: a replay is expected to reproduce on the
// machine and build that made it, not across platforms.

#include "input/InputEvent.h"

namespace weev
{

class WvInputRecording
{
public:
	static const uint32_t RECORDING_MAX_FRAMES = 60 * 60 * 10;
	static const uint32_t RECORDING_MAX_EVENTS = 1 << 18;

	struct FrameRecord
	{
		uint64_t m_uqwStepMicroseconds;
		uint64_t m_uqwStateHash;
		uint32_t m_udwFirstEvent;
		uint32_t m_udwEventCount;
	};

	WvInputRecording();
	~WvInputRecording();

	WvInputRecording(const WvInputRecording&) = delete;
	WvInputRecording& operator=(const WvInputRecording&) = delete;

	void Reset();

	// False when full; the frame is then not recorded.
	bool AddFrame(
		uint64_t uqwStepMicroseconds,
		const WvInputEvent* psEvents,
		uint32_t udwEventCount,
		uint64_t uqwStateHash
	);

	uint32_t GetFrameCount() const;
	const FrameRecord& GetFrame(uint32_t udwFrame) const;
	const WvInputEvent* GetEvents(const FrameRecord& sFrame) const;

	bool Save(const char* pszPath) const;
	bool Load(const char* pszPath);

private:
	struct Header
	{
		uint32_t m_udwMagic;
		uint32_t m_udwVersion;
		uint32_t m_udwEventSize;
		uint32_t m_udwFrameCount;
		uint32_t m_udwEventCount;
		uint32_t m_udwReserved;
	};

	static const uint32_t RECORDING_MAGIC = 0x52495657; // "WVIR"
	static const uint32_t RECORDING_VERSION = 1;

	FrameRecord* m_psFrames;
	WvInputEvent* m_psEvents;
	uint32_t m_udwFrameCount = 0;
	uint32_t m_udwEventCount = 0;
};

} // namespace weev
