
#include "Engine.h"
#include "Module.h"
#include "Object/Object.h"
#include "Registry/FileSystem.h"
#include "Window.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Console.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderScene.h"
#include "Game/World.h"
#include "Game/Events.h"
#include "Game/GameInstance.h"
#include "Game/Input/InputManager.h"
#include "Game/Components/CameraComponent.h"
#include "Resources/Material.h"
#include "Resources/Scene.h"
#include "Misc/Timer.h"

#include <GLFW/glfw3.h>
#include <Util/Assert.h>
#include <Util/KeyValue.h>

#ifdef _WIN32
#include "Platform/Windows/DirectX/DirectXRenderer.h"
#include <windows.h>
#endif

#define RENDER_MULTITHREADED 0

CModule& GetModule_Engine();

CEngine* gEngine = nullptr;

bool gIsClient = 0;
bool gIsServer = 0;
bool gIsRunning = 0;
bool gIsEditor = 0;
bool gIsMainGaurded = 0;

void CEngine::InitMinimal()
{
	THORIUM_ASSERT(gEngine, "");

	MakeIndestructible();
	CConsole::Init();
	CONSOLE_LogInfo("CEngine", "Initializing...");
	
	CResourceManager::Init();
	CModuleManager::RegisterModule(&GetModule_Engine());

	if (gIsEditor || !FFileHelper::DirectoryExists(L".\\core"))
	{
		WString enginePath = OSGetEnginePath(ENGINE_VERSION);
		
		if (!enginePath.IsEmpty())
			engineMod = CFileSystem::MountMod(enginePath + L"\\content", L"Engine", enginePath + L"\\sdk_content");
	}
	
	if (!engineMod && FFileHelper::DirectoryExists(L".\\core"))
		engineMod = CFileSystem::MountMod(L".\\core", L"Engine");

	THORIUM_ASSERT(engineMod != nullptr, "Failed to find Engine content!");
	
	bInitialized = true;
}

void CEngine::Init()
{
	InitMinimal();
	LoadProject();

	CWindow::Init();

	LoadUserConfig();

	gameWindow = new CWindow(userConfig.windowWidth, userConfig.windowHeight, userConfig.windowPosX, userConfig.windowPosY, activeGame.title, (CWindow::EWindowMode)userConfig.windowMode);
#ifdef _WIN32
	Renderer::CreateRenderer<DirectXRenderer>();
#endif

	gameWindow->swapChain = gRenderer->CreateSwapChain(gameWindow);

	inputManager->SetInputWindow(gameWindow);

	//worldRenderScene = new CRenderScene();
	//worldRenderScene->SetFrameBuffer(gameWindow->swapChain->GetFrameBuffer());
	//worldRenderScene->SetDepthBuffer(gameWindow->swapChain->GetDepthBuffer());
	//worldRenderScene->SetCamera(CreateObject<CCameraComponent>());

	LoadWorld(ToWString(activeGame.startupScene));
}

void CEngine::LoadGame()
{
	WString wGame = ToWString(projectConfig.game);
	WString kvPath = wGame + L"\\config\\gameinfo.cfg";
	FKeyValue gameinfo(kvPath);
	THORIUM_ASSERT(gameinfo.IsOpen(), FString("Failed to open '") + ToFString(kvPath) + "'");

#if CONFIG_Debug
	WString libPath = wGame + L"\\bin\\Debug\\" + wGame + L".dll";
#endif
#if CONFIG_Development
	WString libPath = wGame + L"\\bin\\Development\\" + wGame + L".dll";
#endif
#if CONFIG_Release
	WString libPath = wGame + L"\\bin\\" + wGame + L".dll";
#endif
	int r = CModuleManager::LoadModule(libPath);
	if (r == 1)
		CONSOLE_LogWarning("CEngine", FString("Failed to locate module for mod '") + projectConfig.game + "'");
	else if (r > 1)
		CONSOLE_LogError("CEngine", FString("Failed to initialize module '") + projectConfig.game + "'");

	FMod* fMod = CFileSystem::MountMod(wGame);
	WString sdkPath = L".project\\" + wGame + L"\\sdk_content";
	if (FFileHelper::DirectoryExists(sdkPath))
		fMod->SetSdkPath(sdkPath);

	activeGame.name = projectConfig.game;
	activeGame.mod = fMod;
	activeGame.title = *gameinfo.GetValue("title");
	activeGame.version = *gameinfo.GetValue("version");
	activeGame.startupScene = *gameinfo.GetValue("scene");
	activeGame.gameInstanceClass = gameinfo.GetValue("gameinstance")->Value;
}

void CEngine::LoadWorld(const WString& scene, bool bImmediate)
{
	if (!nextSceneName.IsEmpty())
		return;

	if (scene.IsEmpty())
		return;

	nextSceneName = scene;

	if (bImmediate && !CWorld::IsInUpdate())
		DoLoadWorld();
}

int CEngine::Run()
{
	gIsRunning = true;

	deltaTime = 0.02;
	while (gIsRunning)
	{
		FTimer dtTimer;

		inputManager->ClearCache();
		CWindow::PollEvents();

		inputManager->BuildInput();

		CResourceManager::Update();
		CObjectManager::Update();

		if (!nextSceneName.IsEmpty())
		{
			DoLoadWorld();
			gWorld->Start();
		}

		/*_time = glfwGetTime();
		deltaTime = _time - _prevTime;
		_prevTime = _time;*/

		FTimer updateTimer;

		Events::OnUpdate.Invoke();
		gWorld->Update(deltaTime);

		Events::PostUpdate.Invoke();

		updateTimer.Stop();
		updateTime = updateTimer.GetMiliseconds();

		updateTimer.Begin();

#if RENDER_MULTITHREADED
		gRenderer->JoinRenderThread();
#endif
		gRenderer->BeginRender();

		Events::OnRender.Invoke();

		gWorld->renderScene->SetFrameBuffer(gameWindow->swapChain->GetFrameBuffer());
		gWorld->renderScene->SetDepthBuffer(gameWindow->swapChain->GetDepthBuffer());

		gWorld->Render();
		gRenderer->PushScene(gWorld->renderScene);

#if RENDER_MULTITHREADED
		gRenderer->RenderMT();
#else
		gRenderer->Render();
#endif
		updateTimer.Stop();
		renderTime = updateTimer.GetMiliseconds();

		gameWindow->Present(userConfig.bVSync, 0);

		dtTimer.Stop();
		deltaTime = dtTimer.GetSeconds();

		if (gameWindow->WantsToClose() || bWantsToExit)
			gIsRunning = false;
	}

	OnExit();
	return 0;
}

bool CEngine::LoadProject(const WString& path /*= "."*/)
{
	if (!LoadProjectConfig(path))
		return false;

	CFileSystem::SetCurrentPath(path);
	CConsole::LoadConfig();

	LoadGame();
	LoadCoreAddons();

	if (!activeGame.gameInstanceClass.Get())
	{
		CONSOLE_LogError("CEngine", "No GameInstance class was specified, reverting to default.");
		SetGameInstance<CGameInstance>();
	}
	else
		SetGameInstance(activeGame.gameInstanceClass.Get());

	// TODO: Make input manager class a config variable.
	inputManager = CreateObject<CInputManager>();
	inputManager->LoadConfig();

	return true;
}

void CEngine::LoadAddon(const FAddon& addon)
{

}

void CEngine::Exit()
{
	bWantsToExit = true;
}

void CEngine::OnExit()
{
	SaveUserConfig();
	SaveConsoleLog();

	delete gWorld;
	delete gRenderer;
	delete gameWindow;

	CWindow::Shutdown();

	//for (auto obj : CObjectManager::GetAllObjects())
	//	delete obj.second;
	
	CConsole::Shutdown();
	CResourceManager::Shutdown();
	CModuleManager::Cleanup();
}

void CEngine::DoLoadWorld()
{
	if (gWorld)
		gWorld->Delete();

	gWorld = CreateObject<CWorld>();
	gWorld->MakeIndestructible();

	CONSOLE_LogInfo("CEngine", "Loading scene '" + ToFString(nextSceneName) + "'");

	Events::LevelChange.Invoke();

	if (nextSceneName != L"empty")
	{
		CScene* pScene = nullptr;
		pScene = CResourceManager::GetResource<CScene>(nextSceneName);

		if (!pScene)
			CONSOLE_LogError("CEngine", FString("Failed to find scene file '") + ToFString(nextSceneName) + "'");
		else
			gWorld->LoadScene(pScene);
	}
	else
		gWorld->LoadScene(CreateObject<CScene>());

	gWorld->InitWorld(CWorld::InitializeInfo().RegisterForRendering(false));
	//gWorld->SetRenderScene(worldRenderScene);

	Events::PostLevelChange.Invoke();

	nextSceneName = L"";
}

bool CEngine::LoadProjectConfig(const WString& path)
{
	FKeyValue kv(path + L"\\config\\project.cfg");
	if (!kv.IsOpen())
		return false;

	projectConfig.dir = path;
	projectConfig.name = *kv.GetValue("name");
	projectConfig.dispalyName = *kv.GetValue("displayName");
	projectConfig.author = *kv.GetValue("author");
	projectConfig.game = *kv.GetValue("game");
	
	projectConfig.bIncludesSdk = kv.GetValue("hasSdk")->AsBool();
	projectConfig.bHasEngineContent = kv.GetValue("hasEngineContent")->AsBool();

	if (auto* arr = kv.GetArray("addons"); arr)
		projectConfig.addons = *arr;

	return true;
}

bool CEngine::LoadUserConfig()
{
	FKeyValue kv(ToWString(activeGame.name) + L"\\config\\user.cfg");
	if (!kv.IsOpen())
		return false;

	userConfig.windowPosX = kv.GetValue("window.x")->AsInt();
	userConfig.windowPosY = kv.GetValue("window.y")->AsInt();
	userConfig.windowWidth = kv.GetValue("window.w")->AsInt();
	userConfig.windowHeight = kv.GetValue("window.h")->AsInt();
	if (auto* mode = kv.GetValue("window.mode", false); mode != nullptr)
		userConfig.windowMode = FMath::Clamp(mode->AsInt(), 0, 3);

	userConfig.bVSync = kv.GetValue("vsync")->AsBool();

	if (userConfig.windowWidth == 0 || userConfig.windowHeight == 0)
	{
		userConfig.windowWidth = 1920;
		userConfig.windowHeight = 1080;
	}

	if (userConfig.windowPosX == 0 && userConfig.windowPosY == 0)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		userConfig.windowPosX = mode->width - userConfig.windowWidth / 2;
		userConfig.windowPosY = mode->height - userConfig.windowHeight / 2;
	}

	return true;
}

void CEngine::LoadCoreAddons()
{
	FDirectory* cAddonsDir = engineMod->FindDirectory(L"addons");
	if (!cAddonsDir)
		return;

	for (auto* dir : cAddonsDir->GetSubDirectories())
		RegisterAddon(engineMod->Path() + L"\\" + dir->GetPath());
}

void CEngine::RegisterAddon(const WString& path)
{
	FKeyValue cfg(path + L"\\addon.cfg");
	if (!cfg.IsOpen())
		return;

	FAddon addon{};
	addon.identity = *cfg.GetValue("identity");
	addon.name = *cfg.GetValue("name");
	//addon.type = *cfg.GetValue("type");
	
	addon.bHasContent = cfg.GetValue("hasContent")->AsBool();
	addon.bShipSource = cfg.GetValue("shipSource")->AsBool();

	addon.description = *cfg.GetValue("description");
	addon.category = *cfg.GetValue("category");
	addon.author = *cfg.GetValue("author");

	if (auto* arr = cfg.GetArray("dependencies"); arr)
	{
		for (auto& value : *arr)
		{
			FString type;
			FString name;

			SizeType sep = value.FindFirstOf(':');
			if (sep == -1)
				continue;

			type = value;
			type.Erase(type.begin() + sep, type.end());
			name = value;
			name.Erase(type.begin(), type.begin() + sep + 1);

			FDependency::EType eType = FDependency::INVALID;

			if (type == "library")
				eType = FDependency::LIBRARY;
			else if (type == "addon")
				eType = FDependency::ADDON;
			else if (type == "game_addon")
				eType = FDependency::GAME_ADDON;
			else
				continue;

			addon.dependencies.Add({ eType, name });
		}
	}
}

void CEngine::SaveUserConfig()
{
	FKeyValue kv(ToWString(activeGame.name) + L"\\config\\user.cfg");
	
	if (gameWindow)
	{
		if (gameWindow->GetWindowMode() == CWindow::WM_WINDOWED)
			gameWindow->UpdateWindowRect();

		kv.GetValue("window.x")->Set(FString::ToString(gameWindow->WindowedRect.x));
		kv.GetValue("window.y")->Set(FString::ToString(gameWindow->WindowedRect.y));
		kv.GetValue("window.w")->Set(FString::ToString(gameWindow->WindowedRect.w));
		kv.GetValue("window.h")->Set(FString::ToString(gameWindow->WindowedRect.h));
		kv.GetValue("window.mode")->Set(FString::ToString((int)gameWindow->GetWindowMode()));
		kv.GetValue("vsync")->Set(FString::ToString((int)userConfig.bVSync));
	}

	kv.Save();
}

void CEngine::SaveConsoleLog()
{
	CFStream stream("log.txt", "wb");
	if (!stream.IsOpen())
		return;

#if CONSOLE_USE_ARRAY
	for (auto& l : CConsole::GetMsgCache())
	{
		FConsoleMsg* log = &l;
#else
	FConsoleMsg* log = CConsole::GetLinkedList();
	while (log)
	{
#endif
		FString typeStr;

		char timeBuff[48];

		ctime_s(timeBuff, sizeof(timeBuff), (time_t*)&log->time);
		FString timeStr = timeBuff;
		timeStr.Erase(timeStr.last());

		typeStr = "[" + timeStr + "]";

		if (!log->module.IsEmpty() && log->type != CONSOLE_PLAIN)
		{
			typeStr += '[';
			typeStr += log->module + "] ";
		}

		switch (log->type)
		{
		case CONSOLE_WARNING:
			typeStr += "WARNING: ";
			break;
		case CONSOLE_ERROR:
			typeStr += "ERROR: ";
			break;
		}

		stream.Write(typeStr.Data(), typeStr.Size());
		stream.Write(log->msg.Data(), log->msg.Size());
		stream.Write((void*)"\n", 1);

#if !CONSOLE_USE_ARRAY
		log = log->next;
#endif
	}

	stream.Close();
}

#ifdef IS_DEV
void CEngine::HotReloadModule(const FString& module)
{
	if (!CModuleManager::IsModuleLoaded(module))
		return;



}
#endif

WString CEngine::OSGetEnginePath(const FString& version)
{
#ifdef _WIN32
	WString engineVersion = ToWString(version);
	WString keyPath = WString(L"SOFTWARE\\ThoriumEngine\\") + engineVersion;

	HKEY hKey;
	LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return L"";

	WCHAR strBuff[MAX_PATH];
	DWORD buffSize = sizeof(strBuff);
	lRes = RegQueryValueExW(hKey, L"path", 0, NULL, (LPBYTE)strBuff, &buffSize);
	if (lRes != ERROR_SUCCESS)
		return L"";

	return WString(strBuff);
#endif
}

CGameInstance* CEngine::SetGameInstance(FClass* type)
{
	if (gameInstance)
		gameInstance->Delete();

	gameInstance = (CGameInstance*)CreateObject(type);
	gameInstance->Init();
	return gameInstance;
}
