
#ifdef _WIN32
#include <Windows.h>
#endif

#include <Util/Core.h>
#include "Build.h"

int main(int argc, char** argv)
{
	int targetFunction = 0; // 1 == Build Project, 2 == Generate IDE Project, 3 == Generate IDE Project & Solution
	FString sourcePath;
	FString engineVersion;
	EPlatform targetPlatform = PLATFORM_WIN64;
	EConfig targetConfig = CONFIG_DEBUG;
	ECompiler targetCompiler = COMPILER_MSVC15;

	if (argc > 1)
		sourcePath = argv[1];

	for (int i = 2; i < argc; i++)
	{
		bool bLastArg = i == argc - 1;
		FString arg = argv[i];
		if (arg == "-build")
			targetFunction = 1;
		else if (arg == "-genproj")
			targetFunction = 2;
		else if (arg == "-gensln")
			targetFunction = 3;

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
			targetCompiler = COMPILER_MSVC15;
		else if (arg == "-msvc16")
			targetCompiler = COMPILER_MSVC16;
		else if (arg == "-msvc17")
			targetCompiler = COMPILER_MSVC17;
		else if (arg == "-gcc")
			targetCompiler = COMPILER_GCC;
		else if (arg == "-clang")
			targetCompiler = COMPILER_CLANG;
	}
	
	if (targetFunction == 1)
	{
		FCompileConfig cfg;
		cfg.compiler = targetCompiler;
		cfg.config = targetConfig;
		cfg.platform = targetPlatform;
		cfg.engineVersion = engineVersion;
		cfg.path = sourcePath;

		return CompileSource(cfg);
	}
	if (targetFunction == 2)
	{

	}

	return 1;
}
