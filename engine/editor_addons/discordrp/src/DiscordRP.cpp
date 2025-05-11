
#include "EditorAddon.h"
#include "Console.h"
#include "Engine.h"
#include "Game/World.h"
#include "Assets/Scene.h"

#include "discord_rpc.h"

class CDiscordRPModule : public FEditorModule
{
public:
	CDiscordRPModule() : FEditorModule("discordrp")
	{
	}

	void Init() override
	{
		CONSOLE_LogInfo("CDiscordRPModule", "Hellorld!");

		DiscordEventHandlers eventHandlers{};
		Discord_Initialize("1276577546149953577", &eventHandlers, 0, nullptr);
	}

	void Update() override
	{
		runtime += gEngine->DeltaTime();

		FString status = "Project: " + (gEngine->IsProjectLoaded() ? gEngine->GetProjectConfig().displayName : "None") + "\n";
		bool bSceneFile = gWorld->GetScene() ? gWorld->GetScene()->File() != nullptr : false;

		status += "Scene: " + (bSceneFile ? gWorld->GetScene()->File()->Path() : "New Scene") + "\n";

		DiscordRichPresence rpc {0};
		rpc.details = status.c_str();
		
		Discord_UpdatePresence(&rpc);
	}

	void Shutdown() override
	{
		Discord_ClearPresence();
		Discord_Shutdown();
	}

	double runtime = 0;

} static discordRPModule;

REGISTER_EDITOR_ADDON(discordRPModule)
