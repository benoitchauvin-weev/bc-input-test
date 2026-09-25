#pragma once

// Platform key codes to WvKey, as pure tables: no OS calls, so the test
// checks them directly on any platform.

#include "input/InputTypes.h"

namespace weev
{

class WvInputKeyMap
{
public:
	// A Windows scancode (set 1, lParam bits 16-23), its extended flag
	// (bit 24) and virtual key. The virtual key settles the two cases a
	// scancode cannot: Pause and NumLock share 0x45.
	static WvKey FromWinScancode(
		uint32_t udwScancode, bool bExtended, uint32_t udwVirtualKey
	);

	// A DOM KeyboardEvent.code string, such as "KeyA". KEY_NONE for ""
	// and "Unidentified".
	static WvKey FromDomCode(const char* pszCode);

	static const char* GetName(WvKey eKey);

private:
	// One row per key: its DOM code and a short name for display.
	struct KeyEntry
	{
		WvKey m_eKey;
		const char* m_pszDomCode;
		const char* m_pszName;
	};

	static WvKey FromWinBaseScancode(uint32_t udwScancode);
	static WvKey FromWinExtendedScancode(uint32_t udwScancode);

	static const KeyEntry s_asKeys[];
	static const uint32_t s_udwKeyCount;
};

} // namespace weev
