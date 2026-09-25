#include "core/Params.h"

#include <cstdlib>
#include <cstring>

namespace weev
{

int WvParams::s_nArgCount = 0;
char** WvParams::s_ppszArgs = nullptr;

void WvParams::Init(const int nArgCount, char** const ppszArgs)
{
	s_nArgCount = nArgCount;
	s_ppszArgs = ppszArgs;
}

// Returns the text after "--name", which is "" for a bare flag or
// "=value" for a valued one; nullptr when absent.
const char* WvParams::Find(const char* const pszName)
{
	const size_t uNameLength = std::strlen(pszName);

	for (int i = 1; i < s_nArgCount; i++)
	{
		const char* const pszArg = s_ppszArgs[i];

		if (pszArg[0] != '-' || pszArg[1] != '-')
		{
			continue;
		}

		if (std::strncmp(pszArg + 2, pszName, uNameLength) != 0)
		{
			continue;
		}

		const char chNext = pszArg[2 + uNameLength];

		if (chNext == '\0' || chNext == '=')
		{
			return pszArg + 2 + uNameLength;
		}
	}

	return nullptr;
}

bool WvParams::Has(const char* const pszName)
{
	return Find(pszName) != nullptr;
}

const char* WvParams::GetString(
	const char* const pszName, const char* const pszDefault
)
{
	const char* const pszFound = Find(pszName);

	if (pszFound == nullptr || pszFound[0] != '=')
	{
		return pszDefault;
	}

	return pszFound + 1;
}

int64_t WvParams::GetInt(const char* const pszName, const int64_t qwDefault)
{
	const char* const pszValue = GetString(pszName, nullptr);

	if (pszValue == nullptr)
	{
		return qwDefault;
	}

	return std::strtoll(pszValue, nullptr, 10);
}

} // namespace weev
