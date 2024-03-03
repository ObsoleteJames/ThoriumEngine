#pragma once

#include "EngineCore.h"

#ifdef MGS_DLL
#define MGS_API __declspec(dllexport)
#else
#define MGS_API __declspec(dllimport)
#endif
