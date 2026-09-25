#pragma once

// Command-line parameters, --name or --name=value. A far smaller cousin
// of the engine's WV_PARAM_* macros.

#include <cstdint>

namespace weev
{

class WvParams
{
public:
	static void Init(int nArgCount, char** ppszArgs);

	static bool Has(const char* pszName);
	static const char* GetString(const char* pszName, const char* pszDefault);
	static int64_t GetInt(const char* pszName, int64_t qwDefault);

private:
	static const char* Find(const char* pszName);

	static int s_nArgCount;
	static char** s_ppszArgs;
};

} // namespace weev
