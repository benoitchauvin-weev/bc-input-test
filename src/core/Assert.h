#pragma once

// A minimal assert for the sandbox. Debug builds print and break; release
// builds compile the check out.

#include <cstdio>

#if WV_DEBUG
#define WV_ASSERT(Condition, pszMessage)  \
	do                                    \
	{                                     \
		if (!(Condition))                 \
		{                                 \
			std::fprintf(                 \
				stderr,                   \
				"ASSERT %s:%d (%s) %s\n", \
				__FILE__,                 \
				__LINE__,                 \
				#Condition,               \
				pszMessage                \
			);                            \
			__debugbreak();               \
		}                                 \
	} while (false)
#else
#define WV_ASSERT(Condition, pszMessage) \
	do                                   \
	{                                    \
		(void)sizeof(Condition);         \
	} while (false)
#endif // WV_DEBUG
