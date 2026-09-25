#pragma once

// The one queue every input goes through. Portable: backends, tests and
// agents all push here, and the app drains it once per frame.
//
// Motion never takes a queue slot. Mouse delta and wheel are summed, and
// positions and axes keep their latest value, per device, index, control
// and source, so a fast mouse cannot fill the queue.
//
// When presses and releases alone fill it, further edges are folded into
// a count per control, so no press is ever lost. Anything that fits in
// neither is dropped and counted.

#include "input/InputEvent.h"

namespace weev
{

class WvInputQueue
{
public:
	static const uint32_t QUEUE_CAPACITY = 256;
	static const uint32_t MERGED_CAPACITY = 32;
	static const uint32_t FOLDED_CAPACITY = 64;

	// Frame zero means "the next frame drained".
	static const uint64_t QUEUE_NEXT_FRAME = 0;

	void Reset();

	void Push(const WvInputEvent& sEvent, uint64_t uqwTargetFrame);

	// Copies out every event due at or before this frame, in arrival
	// order, then the folded edges, then the merged motion. Events aimed
	// at a later frame stay queued. Returns the number written.
	uint32_t Drain(
		uint64_t uqwFrameIndex, WvInputEvent* psOut, uint32_t udwOutMax
	);

	// Dropped, because neither the queue nor the fold table had room.
	uint32_t GetDroppedCount() const;

	// Folded into counts, because the queue was full.
	uint32_t GetFoldedCount() const;

	uint32_t GetPendingCount() const;

private:
	struct QueuedEvent
	{
		WvInputEvent m_sEvent;
		uint64_t m_uqwTargetFrame;
	};

	struct MergedMotion
	{
		WvInputEvent m_sEvent;
	};

	// A run of alternating edges on one control, starting with the
	// first one seen. A physical control alternates press and release,
	// so the first edge and the count rebuild the whole run.
	struct FoldedEdges
	{
		WvInputEvent m_sFirst;
		uint32_t m_udwEdgeCount;
	};

	static bool IsMergeable(const WvInputEvent& sEvent);
	static bool IsSameControl(const WvInputEvent& sA, const WvInputEvent& sB);

	void PushMerged(const WvInputEvent& sEvent);
	bool PushFolded(const WvInputEvent& sEvent);

	QueuedEvent m_asQueued[QUEUE_CAPACITY] = {};
	MergedMotion m_asMerged[MERGED_CAPACITY] = {};
	FoldedEdges m_asFolded[FOLDED_CAPACITY] = {};
	uint32_t m_udwQueuedCount = 0;
	uint32_t m_udwMergedCount = 0;
	uint32_t m_udwFoldedEntryCount = 0;
	uint32_t m_udwDroppedCount = 0;
	uint32_t m_udwFoldedCount = 0;
};

} // namespace weev
