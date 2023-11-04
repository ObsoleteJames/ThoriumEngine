#pragma once

#include <Util/Core.h>
#include <Util/KeyValue.h>

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

enum ETarget
{
	TARGET_NONE,
	TARGET_ENGINE,
	TARGET_PROJECT,
	TARGET_ADDON,
	TARGET_BUILD_CFG
};

enum EBuildType
{
	BUILD_ENGINE,
	BUILD_GAME,
	BUILD_LIBRARY
};

struct FCompileConfig
{
	FString path; // build.cfg path
	EPlatform platform;
	EConfig config;
	ECompiler compiler;

	FString engineVersion;
};

int CompileSource(const FCompileConfig& config);

void CopyHeaders(FKeyValue& buildCfg, const FString& source, const FString& out);
void CopyBinaries(const FCompileConfig& config, const FString& out);

bool GenerateBuildFromProject(const FString& projectCfg);
