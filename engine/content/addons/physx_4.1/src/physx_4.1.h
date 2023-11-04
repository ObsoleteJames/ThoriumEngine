#pragma once

#ifdef _WIN32
	#ifdef PHYSX_4_1_DLL
		#define PHYSX_4_1_API __declspec(dllexport)
	#else
		#define PHYSX_4_1_API __declspec(dllimport)
	#endif
#elif __GNUC__
	#ifdef PHYSX_4_1_DLL
		#define PHYSX_4_1_API __atribute__((visibility("default")))
	#else
		#define PHYSX_4_1_API
	#endif
#else
	#define PHYSX_4_1_API
#endif
