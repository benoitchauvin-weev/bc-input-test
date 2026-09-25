#include "input/InputRecording.h"

#include "core/Assert.h"

#include <cstdio>

namespace weev
{

WvInputRecording::WvInputRecording()
{
	// Allocated once, up front, so recording never allocates per frame.
	m_psFrames = new FrameRecord[RECORDING_MAX_FRAMES];
	m_psEvents = new WvInputEvent[RECORDING_MAX_EVENTS];
}

WvInputRecording::~WvInputRecording()
{
	delete[] m_psFrames;
	delete[] m_psEvents;
}

void WvInputRecording::Reset()
{
	m_udwFrameCount = 0;
	m_udwEventCount = 0;
}

bool WvInputRecording::AddFrame(
	const uint64_t uqwStepMicroseconds,
	const WvInputEvent* const psEvents,
	const uint32_t udwEventCount,
	const uint64_t uqwStateHash
)
{
	if (m_udwFrameCount >= RECORDING_MAX_FRAMES ||
		m_udwEventCount + udwEventCount > RECORDING_MAX_EVENTS)
	{
		return false;
	}

	FrameRecord& sFrame = m_psFrames[m_udwFrameCount];
	sFrame.m_uqwStepMicroseconds = uqwStepMicroseconds;
	sFrame.m_uqwStateHash = uqwStateHash;
	sFrame.m_udwFirstEvent = m_udwEventCount;
	sFrame.m_udwEventCount = udwEventCount;

	for (uint32_t i = 0; i < udwEventCount; i++)
	{
		WvInputEvent& sStored = m_psEvents[m_udwEventCount + i];
		sStored = psEvents[i];
		sStored.m_uqwTicks = 0;
	}

	m_udwEventCount += udwEventCount;
	m_udwFrameCount++;
	return true;
}

uint32_t WvInputRecording::GetFrameCount() const
{
	return m_udwFrameCount;
}

const WvInputRecording::FrameRecord& WvInputRecording::GetFrame(
	const uint32_t udwFrame
) const
{
	WV_ASSERT(udwFrame < m_udwFrameCount, "recorded frame out of range");
	return m_psFrames[udwFrame];
}

const WvInputEvent* WvInputRecording::GetEvents(const FrameRecord& sFrame) const
{
	return m_psEvents + sFrame.m_udwFirstEvent;
}

// Sandbox only: the engine loads nothing through direct file I/O. The
// same bytes would go through the streaming pipeline there.
bool WvInputRecording::Save(const char* const pszPath) const
{
	FILE* pFile = nullptr;

	if (fopen_s(&pFile, pszPath, "wb") != 0 || pFile == nullptr)
	{
		return false;
	}

	Header sHeader = {};
	sHeader.m_udwMagic = RECORDING_MAGIC;
	sHeader.m_udwVersion = RECORDING_VERSION;
	sHeader.m_udwEventSize = sizeof(WvInputEvent);
	sHeader.m_udwFrameCount = m_udwFrameCount;
	sHeader.m_udwEventCount = m_udwEventCount;

	bool bOk = fwrite(&sHeader, sizeof(sHeader), 1, pFile) == 1;
	bOk = bOk &&
		  fwrite(m_psFrames, sizeof(FrameRecord), m_udwFrameCount, pFile) ==
			  m_udwFrameCount;
	bOk = bOk &&
		  fwrite(m_psEvents, sizeof(WvInputEvent), m_udwEventCount, pFile) ==
			  m_udwEventCount;

	fclose(pFile);
	return bOk;
}

bool WvInputRecording::Load(const char* const pszPath)
{
	Reset();

	FILE* pFile = nullptr;

	if (fopen_s(&pFile, pszPath, "rb") != 0 || pFile == nullptr)
	{
		return false;
	}

	Header sHeader = {};
	bool bOk = fread(&sHeader, sizeof(sHeader), 1, pFile) == 1;

	bOk = bOk && sHeader.m_udwMagic == RECORDING_MAGIC &&
		  sHeader.m_udwVersion == RECORDING_VERSION &&
		  sHeader.m_udwEventSize == sizeof(WvInputEvent) &&
		  sHeader.m_udwFrameCount <= RECORDING_MAX_FRAMES &&
		  sHeader.m_udwEventCount <= RECORDING_MAX_EVENTS;

	bOk = bOk &&
		  fread(
			  m_psFrames, sizeof(FrameRecord), sHeader.m_udwFrameCount, pFile
		  ) == sHeader.m_udwFrameCount;
	bOk = bOk &&
		  fread(
			  m_psEvents, sizeof(WvInputEvent), sHeader.m_udwEventCount, pFile
		  ) == sHeader.m_udwEventCount;

	fclose(pFile);

	if (!bOk)
	{
		Reset();
		return false;
	}

	m_udwFrameCount = sHeader.m_udwFrameCount;
	m_udwEventCount = sHeader.m_udwEventCount;
	return true;
}

} // namespace weev
