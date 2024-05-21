#pragma once

#pragma warning( disable : 4251 )
#pragma warning( disable : 4275 )

#include <Util/Core.h>
#include <new>

#define ENGINE_VERSION "1.0"

#ifndef ENGINE_NO_ASSERT
#define THORIUM_ASSERT(test, msg) UTIL_ASSERT(test, msg)
#else
#define THORIUM_ASSERT(...)
#endif

#if CONFIG_DEBUG
#define CONFIG_NAME "Debug"
#elif CONFIG_DEVELOPMENT
#define CONFIG_NAME "Development"
#elif CONFIG_RELEASE
#define CONFIG_NAME "Release"
#else
#define CONFIG_NAME "INVALID"
#endif

#if PLATFORM_WINDOWS
#define PLATFORM_NAME "win64"
#elif PLATFORM_LINUX
#define PLATOFMR_NAME "linux"
#else
#define PLATOFRM_NAME "INVALID"
#endif

#ifdef _WIN32
	#ifdef ENGINE_DLL
		#define ENGINE_API __declspec(dllexport)
	#else
		#define ENGINE_API __declspec(dllimport)
	#endif
#elif __GNUC__
	#ifdef ENGINE_DLL
		#define ENGINE_API __attribute__ ((visibility ("default")))
	#else
		#define ENGINE_API
	#endif
#else
	#define ENGINE_API
#endif

#ifdef IS_DEV
#define INCLUDE_EDITOR_DATA 1
#endif
