
#include <Util/Core.h>
#include <Util/Assert.h>

#include "Engine.h"
#include "Misc/CommandLine.h"

#ifdef _WIN32
extern "C" __declspec(dllexport) int Launch(const char* cmdLine)
#endif
{
	FCommandLine::Parse(cmdLine);

	bool bDedicated = false;

	gEngine = new CEngine();

	try 
	{
		gIsMainGaurded = true;

		if (bDedicated)
		{
			gEngine->InitMinimal();
			// TODO: Do server stuff

		}
		else
		{
			gIsClient = true;
			gEngine->Init();
		}

		//gEngine->LoadGame(loadGame);
		return gEngine->Run();
	}
	catch (std::exception e)
	{
		UTIL_ASSERT(0, FString("An exception has occurred!\n") + e.what());
	}
	return 1;
}
