#pragma once

// Turns a frame's events into a WvInputFrame. An instance the app owns,
// portable, with no global state, so a test can drive one directly.
//
// Double buffered: Update reads the front snapshot while Build fills the
// back one, and Build swaps them as it returns. A reference returned by
// Build stays valid until the Build after next, which reuses its buffer,
// so everything reading frame N must finish before frame N+2 is built.
//
// Held state lives here, not in the snapshot: which source and which
// keyboard holds each key. Keyboards are merged, so a key is down while
// any of them holds it, and one keyboard letting go of Left cannot
// release it while another still holds it.

#include "input/InputFrame.h"

namespace weev
{

class WvInputFrameBuilder
{
public:
	WvInputFrameBuilder();

	void Reset();

	const WvInputFrame& Build(
		uint64_t uqwFrameIndex,
		uint64_t uqwStepMicroseconds,
		const WvInputEvent* psEvents,
		uint32_t udwEventCount
	);

	const WvInputFrame& GetFront() const;

private:
	static const uint32_t SOURCE_COUNT = uint32_t(WvInputSource::SOURCE_COUNT);

	void BeginFrame(
		WvInputFrame& oFrame,
		uint64_t uqwFrameIndex,
		uint64_t uqwStepMicroseconds
	) const;
	void Apply(WvInputFrame& oFrame, const WvInputEvent& sEvent);

	void ApplyKey(
		WvInputFrame& oFrame,
		WvKey eKey,
		WvInputSource eSource,
		uint8_t byKeyboard,
		bool bDown
	);
	void ApplyMouseButton(
		WvInputFrame& oFrame,
		uint32_t udwButton,
		WvInputSource eSource,
		bool bDown
	);
	void ApplyPadButton(
		WvInputFrame& oFrame,
		uint32_t udwSlot,
		uint32_t udwButton,
		WvInputSource eSource,
		bool bDown
	);
	void ReleaseSource(WvInputFrame& oFrame, WvInputSource eSource);
	void ReleasePad(WvInputFrame& oFrame, uint32_t udwSlot);

	bool IsKeyHeld(uint32_t udwKey) const;
	bool IsMouseHeld(uint32_t udwButton) const;
	bool IsPadHeld(uint32_t udwSlot, uint32_t udwButton) const;

	static void AddKeyEdge(WvInputFrame& oFrame, uint32_t udwKey, bool bPress);
	static void SetKeyBit(WvInputFrame& oFrame, uint32_t udwKey, bool bDown);
	static void AddSaturating(uint8_t& byCount);

	WvInputFrame m_aoFrames[2];
	uint32_t m_udwFront = 0;

	// Per source: a bit per keyboard index holding each key.
	uint8_t m_aabyKeyHolders[SOURCE_COUNT][INPUT_KEY_COUNT];

	// Per source: a bit per mouse button held.
	uint8_t m_abyMouseHolders[SOURCE_COUNT];

	// Per source and slot: a bit per gamepad button held.
	uint16_t m_aauwPadHolders[SOURCE_COUNT][INPUT_GAMEPAD_SLOT_COUNT];
};

} // namespace weev
