#pragma once

#ifdef MGS_DEMO_DLL
#define MGS_API __declspec(dllexport)
#else
#define MGS_API __declspec(dllimport)
#endif
