#pragma once

#ifdef $(PROJECT_NAME)_DLL
#define $(PROJECT_NAME)_API __declspec(dllexport)
#else
#define $(PROJECT_NAME)_API __declspec(dllimport)
#endif

