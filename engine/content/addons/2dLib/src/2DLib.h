#pragma once

#ifdef _WIN32
	#ifdef TWODLIB_DLL
		#define TWODLIB_API __declspec(dllexport)
	#else
		#define TWODLIB_API __declspec(dllimport)
	#endif
#elif __GNUC__
	#ifdef TWODLIB_DLL
		#define TWODLIB_API __atribute__((visibility("default")))
	#else
		#define TWODLIB_API
	#endif
#else
	#define TWODLIB_API
#endif