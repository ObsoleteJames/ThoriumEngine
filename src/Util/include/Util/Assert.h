#pragma once

#include "Util/Core.h"

void _util_assert(bool test, const FString& funcsig, const FString& message);

#ifndef UTIL_NO_ASSERT
	#define UTIL_ASSERT(test, msg) if (!(test)) _util_assert(test, "" __FUNCSIG__ " Line:" STRINGIZE(__LINE__), msg)
#else
	#define UTIL_ASSERT(...)
#endif
