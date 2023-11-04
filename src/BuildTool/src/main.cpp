
#ifdef _WIN32
#include <Windows.h>
#endif

#include <Util/Core.h>
#include <Util/KeyValue.h>
#include "Build.h"
#include <iostream>

FString GetGameNameFromProject(const FString& projectCfg)
{
	FKeyValue kv(ToWString(projectCfg));
	if (!kv.IsOpen())
		return FString();

	auto* v = kv.GetValue("game", false);
	if (v)
		return *v;

	return FString();
}

int main(int argc, char** argv)
{
	int targetFunction = 1; // 1 == Build Project, 2 == Generate IDE Project, 3 == Generate IDE Project & Solution
	FString sourcePath;
	EPlatform targetPlatform = PLATFORM_WIN64;
	EConfig targetConfig = CONFIG_DEBUG;
	ECompiler targetCompiler = COMPILER_MSVC15;

	// by default the latest version
	FString engineVersion = "1.0";

	FString arg;

	if (argc > 1)
		arg = argv[1];
		//sourcePath = argv[1];

	ETarget target = TARGET_NONE;
	if (auto i = arg.FindLastOf("\\/"); i != -1)
	{
		FString name = arg;
		name.Erase(name.begin(), name.begin() + i + 1);

		if (name == "project.cfg")
			target = TARGET_PROJECT;
		else if (name == "Build.cfg")
			target = TARGET_BUILD_CFG;
	}

	for (int i = 2; i < argc; i++)
	{
		bool bLastArg = i == argc - 1;
		FString arg = argv[i];
		if (arg == "-build")
			targetFunction = 1;
		else if (arg == "-genproj")
			targetFunction = 2;
		//else if (arg == "-gensln")
		//	targetFunction = 3;

		else if (arg == "-engine" && !bLastArg)
			engineVersion = argv[i + 1];

		else if (arg == "-debug")
			targetConfig = CONFIG_DEBUG;
		else if (arg == "-development")
			targetConfig = CONFIG_DEVELOPMENT;
		else if (arg == "-release")
			targetConfig = CONFIG_RELEASE;

		else if (arg == "-x64")
			targetPlatform = PLATFORM_WIN64;
		else if (arg == "-linux")
			targetPlatform = PLATFORM_LINUX;
		else if (arg == "-mac")
			targetPlatform = PLATFORM_MAC;

		else if (arg == "-msvc15")
			targetCompiler = COMPILER_MSVC15; // VS 2017
		else if (arg == "-msvc16")
			targetCompiler = COMPILER_MSVC16; // VS 2019
		else if (arg == "-msvc17")
			targetCompiler = COMPILER_MSVC17; // VS 2020
		else if (arg == "-gcc")
			targetCompiler = COMPILER_GCC;
		else if (arg == "-clang")
			targetCompiler = COMPILER_CLANG;
	}
	
	if (targetFunction == 1)
	{
		if (target == TARGET_PROJECT)
		{
			if (!GenerateBuildFromProject(arg))
			{
				std::cout << "Failed to generate build.cfg for project '" << arg.c_str() << "'\n";
				return 1;
			}

			FString gameName = GetGameNameFromProject(arg);
			sourcePath = arg;
			if (auto i = sourcePath.FindLastOf("\\/"); i != -1)
				sourcePath.Erase(sourcePath.begin() + i, sourcePath.end());
			if (auto i = sourcePath.FindLastOf("\\/"); i != -1)
				sourcePath.Erase(sourcePath.begin() + i, sourcePath.end());

			sourcePath += "\\.project\\" + gameName + "\\Build.cfg";
		}
		else
			sourcePath = arg;

		if (!sourcePath.IsEmpty())
		{
			FCompileConfig cfg;
			cfg.compiler = targetCompiler;
			cfg.config = targetConfig;
			cfg.platform = targetPlatform;
			cfg.engineVersion = engineVersion;
			cfg.path = sourcePath;

			int i = CompileSource(cfg);
			std::cin.get();
			return i;
		}
	}
	if (targetFunction == 2)
	{

	}


	return 1;
}
