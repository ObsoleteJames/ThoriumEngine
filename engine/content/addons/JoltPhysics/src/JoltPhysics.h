#pragma once

#ifdef _WIN32
	#ifdef JOLT_PHYSICS_DLL
		#define JOLTPHYSICS_API __declspec(dllexport)
	#else
		#define JOLTPHYSICS_API __declspec(dllimport)
	#endif
#elif __GNUC__
	#ifdef JOLT_PHYSICS_DLL
		#define JOLTPHYSICS_API __atribute__((visibility("default")))
	#else
		#define JOLTPHYSICS_API
	#endif
#else
	#define JOLTPHYSICS_API
#endif