#include "input/InputQueue.h"

namespace weev
{

void WvInputQueue::Reset()
{
	m_udwQueuedCount = 0;
	m_udwMergedCount = 0;
	m_udwFoldedEntryCount = 0;
	m_udwDroppedCount = 0;
	m_udwFoldedCount = 0;
}

bool WvInputQueue::IsMergeable(const WvInputEvent& sEvent)
{
	return sEvent.m_eType == WvInputEventType::EVENT_MOTION ||
		   sEvent.m_eType == WvInputEventType::EVENT_POSITION ||
		   sEvent.m_eType == WvInputEventType::EVENT_AXIS;
}

bool WvInputQueue::IsSameControl(const WvInputEvent& sA, const WvInputEvent& sB)
{
	return sA.m_eType == sB.m_eType && sA.m_eDevice == sB.m_eDevice &&
		   sA.m_byIndex == sB.m_byIndex && sA.m_uwControl == sB.m_uwControl &&
		   sA.m_eSource == sB.m_eSource;
}

void WvInputQueue::Push(
	const WvInputEvent& sEvent, const uint64_t uqwTargetFrame
)
{
	// Only live motion merges. Motion aimed at a given frame is part of
	// a script and keeps its place in the order.
	if (uqwTargetFrame == QUEUE_NEXT_FRAME && IsMergeable(sEvent))
	{
		PushMerged(sEvent);
		return;
	}

	const bool bLiveButton = sEvent.m_eType == WvInputEventType::EVENT_BUTTON &&
							 uqwTargetFrame == QUEUE_NEXT_FRAME;

	// Once edges have started folding, later edges must fold too, or a
	// release could be queued ahead of the press it follows.
	const bool bMustFold = bLiveButton && m_udwFoldedEntryCount > 0;

	if (!bMustFold && m_udwQueuedCount < QUEUE_CAPACITY)
	{
		QueuedEvent& sQueued = m_asQueued[m_udwQueuedCount];
		sQueued.m_sEvent = sEvent;
		sQueued.m_uqwTargetFrame = uqwTargetFrame;
		m_udwQueuedCount++;
		return;
	}

	if (bLiveButton && PushFolded(sEvent))
	{
		return;
	}

	m_udwDroppedCount++;
}

void WvInputQueue::PushMerged(const WvInputEvent& sEvent)
{
	for (uint32_t i = 0; i < m_udwMergedCount; i++)
	{
		WvInputEvent& sMerged = m_asMerged[i].m_sEvent;

		if (!IsSameControl(sMerged, sEvent))
		{
			continue;
		}

		if (sEvent.m_eType == WvInputEventType::EVENT_MOTION)
		{
			sMerged.m_adwValues[0] += sEvent.m_adwValues[0];
			sMerged.m_adwValues[1] += sEvent.m_adwValues[1];
		}
		else
		{
			sMerged.m_adwValues[0] = sEvent.m_adwValues[0];
			sMerged.m_adwValues[1] = sEvent.m_adwValues[1];
		}

		sMerged.m_uqwTicks = sEvent.m_uqwTicks;
		return;
	}

	if (m_udwMergedCount >= MERGED_CAPACITY)
	{
		m_udwDroppedCount++;
		return;
	}

	m_asMerged[m_udwMergedCount].m_sEvent = sEvent;
	m_udwMergedCount++;
}

bool WvInputQueue::PushFolded(const WvInputEvent& sEvent)
{
	for (uint32_t i = 0; i < m_udwFoldedEntryCount; i++)
	{
		FoldedEdges& sFolded = m_asFolded[i];

		if (IsSameControl(sFolded.m_sFirst, sEvent))
		{
			sFolded.m_udwEdgeCount++;
			m_udwFoldedCount++;
			return true;
		}
	}

	if (m_udwFoldedEntryCount >= FOLDED_CAPACITY)
	{
		return false;
	}

	FoldedEdges& sFolded = m_asFolded[m_udwFoldedEntryCount];
	sFolded.m_sFirst = sEvent;
	sFolded.m_udwEdgeCount = 1;
	m_udwFoldedEntryCount++;
	m_udwFoldedCount++;
	return true;
}

uint32_t WvInputQueue::Drain(
	const uint64_t uqwFrameIndex,
	WvInputEvent* const psOut,
	const uint32_t udwOutMax
)
{
	uint32_t udwWritten = 0;
	uint32_t udwKept = 0;

	for (uint32_t i = 0; i < m_udwQueuedCount; i++)
	{
		const QueuedEvent& sQueued = m_asQueued[i];
		const bool bDue = sQueued.m_uqwTargetFrame <= uqwFrameIndex;

		if (bDue && udwWritten < udwOutMax)
		{
			psOut[udwWritten] = sQueued.m_sEvent;
			udwWritten++;
			continue;
		}

		// Not due yet, or no room this frame: keep it, in order.
		m_asQueued[udwKept] = sQueued;
		udwKept++;
	}

	m_udwQueuedCount = udwKept;

	uint32_t udwFoldedKept = 0;

	for (uint32_t i = 0; i < m_udwFoldedEntryCount; i++)
	{
		FoldedEdges& sFolded = m_asFolded[i];
		WvInputEvent sEdge = sFolded.m_sFirst;

		while (sFolded.m_udwEdgeCount > 0 && udwWritten < udwOutMax)
		{
			psOut[udwWritten] = sEdge;
			udwWritten++;
			sEdge.m_adwValues[0] = sEdge.m_adwValues[0] != 0 ? 0 : 1;
			sFolded.m_udwEdgeCount--;
		}

		if (sFolded.m_udwEdgeCount > 0)
		{
			sFolded.m_sFirst = sEdge;
			m_asFolded[udwFoldedKept] = sFolded;
			udwFoldedKept++;
		}
	}

	m_udwFoldedEntryCount = udwFoldedKept;

	uint32_t udwMergedKept = 0;

	for (uint32_t i = 0; i < m_udwMergedCount; i++)
	{
		if (udwWritten < udwOutMax)
		{
			psOut[udwWritten] = m_asMerged[i].m_sEvent;
			udwWritten++;
			continue;
		}

		m_asMerged[udwMergedKept] = m_asMerged[i];
		udwMergedKept++;
	}

	m_udwMergedCount = udwMergedKept;
	return udwWritten;
}

uint32_t WvInputQueue::GetDroppedCount() const
{
	return m_udwDroppedCount;
}

uint32_t WvInputQueue::GetFoldedCount() const
{
	return m_udwFoldedCount;
}

uint32_t WvInputQueue::GetPendingCount() const
{
	return m_udwQueuedCount + m_udwMergedCount + m_udwFoldedEntryCount;
}

} // namespace weev
