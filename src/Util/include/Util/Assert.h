#pragma once

#include "Util/Core.h"

void _util_assert(bool test, const char* func, int line, const FString& message);

#ifndef UTIL_NO_ASSERT
#	if _WIN32
#		define UTIL_ASSERT(test, msg) if (!(test)) _util_assert(test, __FUNCSIG__, __LINE__, msg)
#	else
#		define UTIL_ASSERT(test, msg) if (!(test)) _util_assert(test, __PRETTY_FUNCTION__, __LINE__, msg)
#	endif
#else
#	define UTIL_ASSERT(...)
#endif
