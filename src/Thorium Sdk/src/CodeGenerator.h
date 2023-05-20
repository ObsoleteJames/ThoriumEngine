#pragma once

#include "EngineCore.h"
#include <Util/Map.h>

class CCodeGenerator
{
public:
	static void GenerateCppFile(const FString& filePath, const FString& baseClass, const WString& mod = WString());
	static void GenerateProjectHeader(const FString& projName, const WString& outPath);

	// Generate CMakeLists.txt for the current game.
	static void GenerateCMakeLists();

	static FString GetIncludePath(const FString& className);

private:
	static FString ParseBaseFile(const WString& file, const TMap<std::string, FString>& variables);

};
