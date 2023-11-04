#pragma once

#include "EngineCore.h"

class ENGINE_API FFileHelper
{
public:
	static bool DirectoryExists(const FString& path);
	static bool FileExists(const FString& path);

	static bool DirectoryExists(const WString& path);
	static bool FileExists(const WString& path);
};
