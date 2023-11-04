#pragma once

#include <Util/Core.h>
#include "EngineCore.h"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define COMMANDLINE_SIZE (MAX_PATH /* * 64*/)

struct ENGINE_API FCommandLine
{
public:
	static void Parse(const char* in_cmd, bool bIgnoreFirst = true);
	static void Parse(const wchar_t* in_cmd, bool bIgnoreFirst = true);
	static void Parse(char** in_cmd, int argc, bool bIngoreFirst = true);

	static void Append(const FString& str);
	static inline bool HasParam(const FString& str) { return FindParam(str) != -1; }
	static SizeType FindParam(const FString& str);
	static inline FString GetParam(SizeType index) { return CmdLine[index]; }

	static inline const TArray<FString>& GetArgs() { return CmdLine; }

	// Returns the argument after the one specified.
	static FString ParamValue(const FString& str);
	static int ParamValue(const FString& str, int defaultVal);
	static float ParamValue(const FString& str, float defautlVal);

private:
	static TArray<FString> CmdLine;

};
