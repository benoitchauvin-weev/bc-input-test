#pragma once

// FNV-1a over raw bytes. Only for types with no padding, or the hash
// would read indeterminate bytes: callers static_assert that first.

#include <cstddef>
#include <cstdint>

namespace weev
{

class WvHash
{
public:
	static const uint64_t HASH_SEED = 0xcbf29ce484222325ull;

	static uint64_t Fnv1a(
		const void* pData, size_t uSize, uint64_t uqwSeed = HASH_SEED
	)
	{
		const uint8_t* const pbyData = (const uint8_t*)pData;
		uint64_t uqwHash = uqwSeed;

		for (size_t i = 0; i < uSize; i++)
		{
			uqwHash ^= pbyData[i];
			uqwHash *= 0x100000001b3ull;
		}

		return uqwHash;
	}
};

} // namespace weev
