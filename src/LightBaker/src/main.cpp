
#include "Engine.h"
#include "Misc/CommandLine.h"
#include "Game/World.h"
#include "LightBaker.h"

#ifdef _WIN32
#include "windows.h"
#endif
#undef max()

#include <iostream>
#include <limits>

class CBakeEngine : public CEngine
{
public:
	inline void Shutdown() { OnExit(); }
};

#ifdef _WIN32
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#else
int main(int argc, char** argv)
#endif
{
#ifdef _WIN32
	FCommandLine::Parse(lpCmdLine, false);
	int argc = 0;
	char** argv = nullptr;
#else
	FCommandLine::Parse(argv, argc);
#endif

	FString scenePath;
	FString projectPath;

	auto& args = FCommandLine::GetArgs();

	for (SizeType i = 0; i < args.Size(); i++)
	{
		if (args[i] == "-scene" && args.Size() > i + 1)
		{
			scenePath = args[i + 1];
			i++;
		}

		if (args[i] == "-project" && args.Size() > i + 1)
		{
			projectPath = args[i + 1];
			i++;
		}
	}

	CConsole::bLoggingEnabled = false;
	gEngine = new CBakeEngine();
	CBakeEngine* bakeEngine = (CBakeEngine*)gEngine;
	gEngine->InitTerminal();
	CConsole::bLoggingEnabled = true;

	if (args.Size() > 0 && args[0].Find(".thasset") != -1)
	{
		scenePath = args[0];
		scenePath.ReplaceAll('\\', '/');
		for (auto* m : CFileSystem::GetMods())
		{
			if (scenePath.Find(m->Path()) != -1)
			{
				scenePath.Erase(scenePath.begin(), scenePath.begin() + m->Path().Size() + 1);
				break;
			}
		}
	}

	if (scenePath.IsEmpty())
	{
		CONSOLE_LogError("Engine", "No scene specified! use -scene <path> to specify a scene to load.");
		bakeEngine->Shutdown();
		std::cout << "press any key to exit..." << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cin.get();
		return 1;
	}

	if (!projectPath.IsEmpty())
		gEngine->LoadProject(projectPath);

	gEngine->LoadWorld(scenePath, true);

	CLightBaker baker(gWorld);
	baker.BakeAll();

	if (baker.GetResult().bSuccess)
	{
		CONSOLE_LogInfo("LightBaker", "Light baking completed successfully!");
		CONSOLE_LogInfo("LightBaker", "Saving light data...");
		baker.SaveData();
	}
	else
	{
		CONSOLE_LogError("LightBaker", "Light baking failed! check log for errors.");
		bakeEngine->Shutdown();
		std::cout << "press any key to exit..." << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cin.get();
		return 1;
	}

	bakeEngine->Shutdown();
	return 0;
}
