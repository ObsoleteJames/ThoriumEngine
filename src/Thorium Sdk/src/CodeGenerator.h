#pragma once

#include "EngineCore.h"
#include <Util/Map.h>

class CCodeGenerator
{
public:
	static void GenerateCppFile(const FString& filePath, const FString& baseClass, const WString& mod = WString());

	static FString GetIncludePath(const FString& className);

private:
	static FString ParseBaseFile(const WString& file, const TMap<std::string, FString>& variables);

};
