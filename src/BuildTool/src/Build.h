#pragma once

#include <Util/Core.h>

enum ECompiler
{
	COMPILER_MSVC15,
	COMPILER_MSVC16,
	COMPILER_MSVC17,
	COMPILER_CLANG,
	COMPILER_GCC
};

enum EPlatform
{
	PLATFORM_WIN64,
	PLATFORM_LINUX,
	PLATFORM_MAC
};

enum EConfig
{
	CONFIG_DEBUG,
	CONFIG_DEVELOPMENT,
	CONFIG_RELEASE
};

struct FCompileConfig
{
	FString path;
	EPlatform platform;
	EConfig config;
	ECompiler compiler;

	FString engineVersion;
};

int CompileSource(const FCompileConfig& config);

void CopyHeaders(const FString& source, const FString& out);
void CopyBinaries(const FString& source, const FString& out);

