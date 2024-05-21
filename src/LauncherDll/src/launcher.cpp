
#include <Util/Core.h>
#include <Util/Assert.h>

#include "Engine.h"
#include "Misc/CommandLine.h"

#ifdef _WIN32
extern "C" __declspec(dllexport) int Launch(const char* cmdLine)
#else
extern "C" __attribute__((visibility("default"))) int Launch(const char* cmdLine)
#endif
{
	FCommandLine::Parse(cmdLine);

	bool bDedicated = false;
	if (FCommandLine::HasParam("-dedicated"))
		bDedicated = true;

	gEngine = new CEngine();

	try 
	{
		gIsMainGaurded = true;

		if (bDedicated)
		{
			//gEngine->InitMinimal();
			gEngine->InitTerminal();
			// TODO: Do server stuff

			//gEngine->HostServer("42708")

		}
		else
		{
			gIsClient = true;
			gEngine->Init();
		}

		//gEngine->LoadGame(loadGame);
		return gEngine->Run();
	}
	catch (std::exception& e)
	{
		UTIL_ASSERT(0, FString("An exception has occurred!\n") + e.what());
	}
	return 1;
}
