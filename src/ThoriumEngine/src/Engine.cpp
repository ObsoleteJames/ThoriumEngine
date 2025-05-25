
#include "Engine.h"
#include "Module.h"
#include "Object/Object.h"
#include "Registry/FileSystem.h"
#include "Window.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Console.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsInterface.h"
#include "Rendering/RenderScene.h"
#include "Rendering/DefaultRenderer.h"
#include "Game/World.h"
#include "Game/Events.h"
#include "Game/GameInstance.h"
#include "Game/Input/InputManager.h"
#include "Game/Components/CameraComponent.h"
#include "Assets/Material.h"
#include "Assets/Scene.h"
#include "Misc/Timer.h"

#include "ImGui/imgui.h"

#include <GLFW/glfw3.h>
#include <Util/Assert.h>
#include <Util/KeyValue.h>

#ifdef _WIN32
#include "Platform/Windows/DirectX/DirectXInterface.h"
#include <windows.h>
#include <shlobj.h>
#endif

#include <filesystem>

#define RENDER_MULTITHREADED 0

CModule& GetModule_Engine();

CEngine* gEngine = nullptr;

bool bIsTerminal = 0; // wether the engine is running in terminal mode
bool gIsClient = 0;
bool gIsServer = 0;
bool gIsRunning = 0;
bool gIsEditor = 0;
bool gIsMainGaurded = 0;

static CConCmd cmdLoadScene("scene", [](const TArray<FString>& args) { gEngine->LoadWorld(args[0]); });
static CConCmd cmdQuit("quit", []() { gEngine->Exit(); });

void CEngine::InitMinimal()
{
	THORIUM_ASSERT(gEngine, "");

	MakeIndestructible();
	CConsole::Init();
	CONSOLE_LogInfo("CEngine", "Initializing...");
	
	CAssetManager::Init();
	CModuleManager::RegisterModule(&GetModule_Engine());

	if (gIsEditor || !FFileHelper::DirectoryExists("./core"))
	{
		FString enginePath = OSGetEnginePath(ENGINE_VERSION);
		
		if (!enginePath.IsEmpty())
			engineMod = CFileSystem::MountMod(enginePath + "/content", "Engine", enginePath + "/sdk_content");
	}
	
	if (!engineMod && FFileHelper::DirectoryExists("./core"))
		engineMod = CFileSystem::MountMod("./core", "Engine");

	THORIUM_ASSERT(engineMod != nullptr, "Failed to find Engine content!");

	engineMod->type = MOD_ENGINE;

	// Fetch core addons
	FetchAddons(engineMod->Path() + "/addons", coreAddons);

	// Load all mandatory addons, these addons are required for the engine to work.
	LoadMandatoryAddons();

	CONSOLE_LogInfo("CEngine", "Initialization complete");
	bInitialized = true;
}

void CEngine::Init()
{
	InitMinimal();
	THORIUM_ASSERT(LoadProject(), "Failed to load project!");

	CWindow::Init();

	LoadUserConfig();

	gameWindow = new CWindow(userConfig.windowWidth, userConfig.windowHeight, userConfig.windowPosX, userConfig.windowPosY, activeGame.title);

	gGHI = GetGraphicsInterface();
	gGHI->Init();

	gRenderer = CreateObject<CDefaultRenderer>();
	gRenderer->MakeIndestructible();
	gRenderer->Init();

	gameWindow->swapChain = gGHI->CreateSwapChain(gameWindow);
	gameWindow->SetWindowMode((CWindow::EWindowMode)userConfig.windowMode);

	// Clear the screen in order to prevent it from being white as the scene loads
	gameWindow->swapChain->GetFrameBuffer()->Clear();
	gameWindow->Present(0, 0);

	if (!inputManager)
		inputManager = CreateObject<CInputManager>();
	inputManager->SetInputWindow(gameWindow);

	InitImGui();

	//worldRenderScene = new CRenderScene();
	//worldRenderScene->SetFrameBuffer(gameWindow->swapChain->GetFrameBuffer());
	//worldRenderScene->SetDepthBuffer(gameWindow->swapChain->GetDepthBuffer());
	//worldRenderScene->SetCamera(CreateObject<CCameraComponent>());

	LoadWorld(activeGame.startupScene);
}

void CEngine::InitTerminal()
{
#if PLATFORM_WINDOWS
	AllocConsole();
	FILE* temp;
	freopen_s(&temp, "CONIN$", "r", stdin);
	freopen_s(&temp, "CONOUT$", "w", stderr);
	freopen_s(&temp, "CONOUT$", "w", stdout);
#endif

	bIsTerminal = true;
	CConsole::EnableStdio();

	InitMinimal();
	THORIUM_ASSERT(LoadProject(), "Failed to load project!");

	LoadWorld(activeGame.startupScene);
}

void CEngine::LoadGame()
{
	FString wGame = projectConfig.game;
	FString kvPath = wGame + "/config/gameinfo.cfg";
	FKeyValue gameinfo(kvPath);
	THORIUM_ASSERT(gameinfo.IsOpen(), "Failed to open '" + kvPath + "'");

// #if CONFIG_Debug
// 	FString libPath = wGame + L"\\bin\\Debug\\" + wGame + L".dll";
// #endif
// #if CONFIG_Development
// 	FString libPath = wGame + L"\\bin\\Development\\" + wGame + L".dll";
// #endif
// #if CONFIG_Release
// 	FString libPath = wGame + L"\\bin\\" + wGame + L".dll";
// #endif
#if CONFIG_Release
	FString libPath = wGame + ("/bin/" PLATFORM_NAME "/") + wGame + ".dll";
#else
	FString libPath = wGame + ("/bin/" PLATFORM_NAME "/" CONFIG_NAME "/") + wGame + ".dll";
#endif

	int r = CModuleManager::LoadModule(libPath, &activeGame.module);
	if (r == 1)
		CONSOLE_LogWarning("CEngine", FString("Failed to locate module for mod '") + projectConfig.game + "'");
	else if (r > 1)
		CONSOLE_LogError("CEngine", FString("Failed to initialize module '") + projectConfig.game + "'");

	FMod* fMod = CFileSystem::MountMod(wGame);
	fMod->type = MOD_GAME;

	FString sdkPath = ".project/" + wGame + "/sdk_content";
	if (FFileHelper::DirectoryExists(sdkPath))
		fMod->SetSdkPath(sdkPath);

	activeGame.name = projectConfig.game;
	activeGame.mod = fMod;
	activeGame.title = *gameinfo.GetValue("title");
	activeGame.version = *gameinfo.GetValue("version");
	activeGame.startupScene = *gameinfo.GetValue("scene");
	activeGame.gameInstanceClass = gameinfo.GetValue("gameinstance")->Value;
	activeGame.inputManagerClass = gameinfo.GetValue("inputmanager")->Value;
	if (activeGame.inputManagerClass.Get() == nullptr)
		activeGame.inputManagerClass = CInputManager::StaticClass();
}

void CEngine::LoadWorld(const FString& scene, bool bImmediate)
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

		CAssetManager::Update();
		CObjectManager::Update();
		CConsole::Update();

		if (!bIsTerminal)
		{
			inputManager->ClearCache();
			CWindow::PollEvents();

			inputManager->BuildInput();

			gGHI->ImGuiBeginFrame();
		}

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
		gWorld->Update(FMath::Min(deltaTime, 0.25));

		Events::PostUpdate.Invoke();

		updateTimer.Stop();
		updateTime = updateTimer.GetMiliseconds();

		//ImGui::ShowDemoWindow();

		if (!bIsTerminal)
		{
#if RENDER_MULTITHREADED
			gRenderer->JoinRenderThread();
#endif
			updateTimer.Begin();

			Events::OnRender.Invoke();

			gWorld->renderScene->SetScreenPercentage(cvRenderScreenPercentage.AsFloat());
			gWorld->renderScene->SetFrameBuffer(gameWindow->swapChain->GetFrameBuffer());
			//gWorld->renderScene->SetDepthBuffer(gameWindow->swapChain->GetDepthBuffer());

			gWorld->Render();
			gRenderer->PushScene(gWorld->renderScene);

#if RENDER_MULTITHREADED
			gRenderer->RenderMT();
#else
			gRenderer->Render();
#endif
			updateTimer.Stop();
			renderTime = updateTimer.GetMiliseconds();

			gameWindow->swapChain->GetDepthBuffer()->Clear();
			gGHI->ImGuiRender();

			gameWindow->Present(userConfig.bVSync, 0);
		}

		dtTimer.Stop();
		deltaTime = dtTimer.GetSeconds();

		if ((gameWindow && gameWindow->WantsToClose()) || bWantsToExit)
			gIsRunning = false;
	}

	if (gGHI)
		gGHI->ImGuiShutdown();

	OnExit();
	return 0;
}

bool CEngine::LoadProject(const FString& path /*= "."*/)
{
	if (bProjectLoaded)
		UnloadProject();

	if (!LoadProjectConfig(path, projectConfig))
		return false;

	CONSOLE_LogInfo("CEngine", "Loading project '" + projectConfig.name + "'");

	CFileSystem::SetCurrentPath(path);
	CConsole::LoadConfig();

	FetchAddons(projectConfig.game + "/addons", projectConfig.projectAddons);

	// Load libraries this project depends on.
	for (auto l : projectConfig.addons)
	{
		bool bCore = false;
		
		if (auto i = l.FindFirstOf(':'); i != -1)
		{
			FString type = l;
			type.Erase(type.begin() + i, type.end());
			l.Erase(l.begin(), l.begin() + i + 1);

			if (type == "core")
				bCore = true;
		}

		if (bCore)
		{
			for (auto& addon : coreAddons)
			{
				if (addon.identity == l)
				{
					LoadAddon(addon);
					break;
				}
			}
		}
		else
		{
			for (auto& addon : projectConfig.projectAddons)
			{
				if (addon.identity == l)
				{
					LoadAddon(addon);
					break;
				}
			}
		}
	}

	LoadGame();

	if (!activeGame.gameInstanceClass.Get())
	{
		CONSOLE_LogError("CEngine", "No GameInstance class was specified, reverting to default.");
		SetGameInstance<CGameInstance>();
	}
	else
		SetGameInstance(activeGame.gameInstanceClass.Get());

	if (!physicsSettings.api.Get())
	{
		CONSOLE_LogWarning("CEngine", "No Physic API class was specified, reverting to default (Jolt Physics).");
		physicsSettings.api = FString("CJoltPhysicsApi");
	}

	CreatePhysicsApi(physicsSettings.api.Get());

	if (!bIsTerminal)
	{
		if (!inputManager || inputManager->GetClass() != activeGame.inputManagerClass.Get())
		{
			TObjectPtr<CInputManager> oldIM = inputManager;
			inputManager = (CInputManager*)CreateObject(activeGame.inputManagerClass.Get());
			if (oldIM)
			{
				inputManager->CopyState(oldIM);
				oldIM->Delete();
			}
			else
				inputManager->LoadConfig();
		}
		else
			inputManager->LoadConfig();
	}

	bProjectLoaded = true;
	return true;
}

bool CEngine::UnloadProject()
{
	if (!bProjectLoaded)
		return false;

	THORIUM_ASSERT(CWorld::IsInUpdate() == false, "Cannot unload project while in update function.");

	// Make sure to unload the current world in case it holds objects that are from the games' module
	UnloadWorld();

	// Replace all customizable objects with default types, since they could be from the games' module.
	SetGameInstance<CGameInstance>();

	physicsSettings.api = FString("CJoltPhysicsApi");
	CreatePhysicsApi(physicsSettings.api.Get());

	TObjectPtr<CInputManager> oldIm = inputManager;
	inputManager = CreateObject<CInputManager>();
	if (oldIm)
	{
		inputManager->CopyState(oldIm);
		oldIm->Delete();
	}

	// Update the object manager to make sure all old objects are safely deleted
	CObjectManager::Update();

	for (auto& a : projectConfig.projectAddons)
		UnloadAddon(a);

	if (!CFileSystem::UnmountMod(activeGame.mod))
		return false;

	// now delete all remaining objects in a not so safe way
	if (activeGame.module)
		CObjectManager::DeleteObjectsFromModule(activeGame.module);

	if (activeGame.module && !CModuleManager::UnloadModule(activeGame.module))
		return false;

	// now load a new world so we don't crash
	LoadWorld("empty", true);

	bProjectLoaded = false;
	return true;
}

void CEngine::LoadAddon(FAddon& addon)
{
	// don't load it if it's already been loaded.
	if (addon.module || addon.mod)
		return;

	CONSOLE_LogInfo("CEngine", "Loading Addon '" + addon.name + "'");

	// first load the dependancies
	for (auto& d : addon.dependencies)
	{
		switch (d.type)
		{
		case FDependency::LIBRARY:
		{
			// since this is a path, we have to truncate it into a name for the LoadFLibrary function.
			// it's not actually necessary but it's just nicer.
			FString libName = d.name;
			if (auto i = libName.FindLastOf("\\/"); i != -1)
				libName.Erase(libName.begin(), libName.begin() + i + 1);
			if (auto i = libName.FindLastOf('.'); i != -1)
				libName.Erase(libName.begin() + i, libName.end());

			if (d.instance = (void*)CModuleManager::LoadFLibrary(libName, addon.path + "/" + d.name); d.instance == nullptr)
			{
				CONSOLE_LogError("CEngine", "Failed to load addon dependency (FDependency::LIBRARY)\n" + addon.path + "/" + d.name);
			}
			break;
		}
		case FDependency::ADDON:
			for (auto& l : coreAddons)
			{
				if (l.identity == d.name)
				{
					LoadAddon(l);
					break;
				}
			}
			break;
		case FDependency::GAME_ADDON:
			for (auto& l : projectConfig.projectAddons)
			{
				if (l.identity == d.name)
				{
					LoadAddon(l);
					break;
				}
			}
			break;
		}
	}

	if (addon.bHasCode)
	{
		// We might need to change this for other platforms.
#if CONFIG_Release
		FString libPath = addon.path + "/bin/" PLATFORM_NAME "/" + addon.identity + ".dll";
#else
		FString libPath = addon.path + "/bin/" PLATFORM_NAME "/" CONFIG_NAME "/" + addon.identity + ".dll";
#endif

		CModule* lib;
		if (int err = CModuleManager::LoadModule(libPath, &lib); err != 0)
		{
			if (err == 1)
				CONSOLE_LogError("CEngine", "Failed to load addon module, file does not exist\n" + libPath);
			else
				CONSOLE_LogError("CEngine", "Failed to load addon module, is the library compiled properly?\n" + libPath);
		}
		addon.module = lib;
	}

	if (addon.bHasContent)
	{
		addon.mod = CFileSystem::MountMod(addon.path + "/content", addon.name, addon.path + "/sdk_content");
		addon.mod->type = MOD_ADDON;
		if (!addon.mod)
		{
			CONSOLE_LogError("CEngine", "Failed to mount library content\n" + addon.path + "/content");
		}
	}
}

void CEngine::UnloadAddon(FAddon& addon)
{
	for (auto& d : addon.dependencies)
	{
		if (d.type == FDependency::LIBRARY && d.instance != nullptr)
		{
			CModuleManager::UnloadLibrary((FLibrary*)d.instance);
			d.instance = nullptr;
		}
	}

	if (addon.module)
	{
		CObjectManager::DeleteObjectsFromModule(addon.module);
		CModuleManager::UnloadModule(addon.module);
	}

	if (addon.mod)
		CFileSystem::UnmountMod(addon.mod);

	addon.module = nullptr; 
	addon.mod = nullptr;
}

void CEngine::LoadCoreAddon(const FString& id)
{
	for (auto& addon : coreAddons)
	{
		if (addon.identity == id)
		{
			LoadAddon(addon);
			break;
		}
	}
}

void CEngine::Exit()
{
	bWantsToExit = true;
}

void CEngine::OnExit()
{
	CONSOLE_LogInfo("CEngine", "Shutting down...");

	SaveUserConfig();

	gWorld->Delete();
	delete gameWindow;

	gPhysicsApi->Shutdown();
	gPhysicsApi->Delete();
	gPhysicsApi = nullptr;

	gameInstance->Delete();
	gameInstance = nullptr;

	// Clear Shader list
	(*(TArray<TObjectPtr<CShaderSource>>*)&CShaderSource::GetAllShaders()).Clear();

	gRenderer->Delete();
	delete gGHI;

	if (!bIsTerminal)
		CWindow::Shutdown();

	CFileSystem::UnmountMod(engineMod);

	CAssetManager::Shutdown();
	CModuleManager::Cleanup();

	CObjectManager::Shutdown();

	SaveConsoleLog();
	CConsole::Shutdown();
}

void CEngine::InitImGui()
{
	if (!gRenderer)
		return;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;	      // Enable Multi-Viewport / Platform Windows
	
	io.IniFilename = "./config/imgui.ini";

	FFile* fontFile = CFileSystem::FindFile("fonts/Roboto-Regular.ttf");

	if (fontFile)
	{
		ImFontConfig cfg;
		//cfg.OversampleH = cfg.OversampleV = 2;
		//cfg.PixelSnapH = true;
		cfg.FontDataOwnedByAtlas = false;

		ImFont* font = io.Fonts->AddFontFromFileTTF(fontFile->FullPath().c_str(), 14, &cfg);
		ImFont* font2 = io.Fonts->AddFontFromFileTTF(fontFile->FullPath().c_str(), 18, &cfg);
		//font->Scale = 0.6f;
		io.Fonts->AddFontDefault();
	}

	gGHI->InitImGui(gameWindow);
}

void CEngine::DoLoadWorld()
{
	CScene* pScene = nullptr;
	if (nextSceneName != "empty")
	{
		pScene = CAssetManager::GetAsset<CScene>(nextSceneName);
		if (!pScene)
		{
			CONSOLE_LogError("CEngine", FString("Failed to find scene file '") + nextSceneName + "'");
			nextSceneName = "empty";
		}
	}

	Events::LevelChange.Invoke();

	if (gWorld)
		gWorld->Delete();

	gWorld = CreateObject<CWorld>();
	gWorld->MakeIndestructible();

	CONSOLE_LogInfo("CEngine", "Loading scene '" + nextSceneName + "'");

	gWorld->InitWorld(CWorld::InitializeInfo().RegisterForRendering(false));

	if (nextSceneName != "empty")
	{
		gWorld->LoadScene(pScene);
	}
	else
		gWorld->LoadScene(CreateObject<CScene>());

	//gWorld->SetRenderScene(worldRenderScene);

	Events::PostLevelChange.Invoke();

	nextSceneName.Clear();
}

bool CEngine::LoadProjectConfig(const FString& path, FProject& project)
{
	FKeyValue kv(path + "/config/project.cfg");
	if (!kv.IsOpen())
		return false;

	project.dir = path;
	project.name = *kv.GetValue("name");
	project.displayName = *kv.GetValue("displayName");
	project.author = *kv.GetValue("author");
	project.game = *kv.GetValue("game");
	
	project.bIncludesSdk = kv.GetValue("hasSdk")->AsBool();
	project.bHasEngineContent = kv.GetValue("hasEngineContent")->AsBool();

	if (auto* arr = kv.GetArray("addons"); arr)
		project.addons = *arr;

	FKeyValue phys(path + "/config/physics.cfg");

	physicsSettings.api = phys.GetValue("api")->Value;
	physicsSettings.gravity = { 0, -9.8f, 0 };

	return true;
}

bool CEngine::LoadUserConfig()
{
	FKeyValue kv(activeGame.name + "/config/user.cfg");
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

void CEngine::FetchAddons(const FString& addonFolder, TArray<FAddon>& out)
{
	// the filesystem ignores the addons folder so this needs to be changed.
	//FDirectory* cAddonsDir = engineMod->FindDirectory(L"addons");
	//if (!cAddonsDir)
	//	return;

	if (!FFileHelper::DirectoryExists(addonFolder))
		return;

	//for (auto* dir : cAddonsDir->GetSubDirectories())
	for (auto entry : std::filesystem::directory_iterator(addonFolder.c_str()))
	{
		if (!entry.is_directory())
			continue;

		FAddon addon;
		FString p = addonFolder + "/" + entry.path().filename().generic_string().c_str();

		if (LoadAddonConfig(p, addon))
			out.Add(addon);
	}
}

bool CEngine::LoadAddonConfig(const FString& path, FAddon& out)
{
	FKeyValue cfg(path + "/addon.cfg");
	if (!cfg.IsOpen())
		return false;

	FAddon addon{};
	addon.identity = *cfg.GetValue("identity");
	addon.name = *cfg.GetValue("name");
	addon.path = path;
	FString addonType = *cfg.GetValue("type");
	addon.type = addonType == "CoreAddon" ? FAddon::CORE_ADDON : (addonType == "GameAddon" ? FAddon::GAME_ADDON : FAddon::INVALID_ADDON);
	
	addon.bHasCode = cfg.GetValue("hasCode")->AsBool();
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
			name.Erase(name.begin(), name.begin() + sep + 1);

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

	out = addon;
	return true;
}

void CEngine::LoadMandatoryAddons()
{
	LoadCoreAddon("jolt_physics");

	CreatePhysicsApi(CModuleManager::FindClass("CJoltPhysicsApi"));
}

void CEngine::UnloadWorld()
{
	if (gWorld)
	{
		gWorld->Delete();
		gWorld = nullptr;
	}
}

void CEngine::SaveUserConfig()
{
	FKeyValue kv(activeGame.name + "/config/user.cfg");
	
	if (gameWindow)
	{
		//if (gameWindow->GetWindowMode() == CWindow::WM_WINDOWED)
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

#if _WIN32
		char timeBuff[48];
		ctime_s(timeBuff, sizeof(timeBuff), (time_t*)&log->time);
#else
		char* timeBuff = ctime((time_t*)&log->time);
#endif

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

#ifdef _WIN32
FString CEngine::OSGetEnginePath(const FString& engineVersion)
{
	FString keyPath = "SOFTWARE\\ThoriumEngine\\" + engineVersion;

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, keyPath.c_str(), 0, KEY_READ, &hKey);
	if (lRes == ERROR_FILE_NOT_FOUND)
		return "";

	CHAR strBuff[MAX_PATH];
	DWORD buffSize = sizeof(strBuff);
	lRes = RegQueryValueEx(hKey, "path", 0, NULL, (LPBYTE)strBuff, &buffSize);
	if (lRes != ERROR_SUCCESS)
		return "";

	return FString(strBuff);
}

FString CEngine::OSGetDataPath()
{
	PWSTR appdata;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appdata)))
		return FString();

	char r[MAX_PATH];
	wcstombs(r, appdata, MAX_PATH);

	return FString(r);
}

FString CEngine::OSGetDocumentsPath()
{
	PWSTR appdata;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &appdata)))
		return FString();

	char r[MAX_PATH];
	wcstombs(r, appdata, MAX_PATH);

	return FString(r);
}

FString CEngine::OpenFileDialog(const FString& filter /*= FString()*/)
{
	OPENFILENAMEA ofn;
	CHAR szFile[255] = { 0 };
	CHAR currentDir[255] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	if (GetCurrentDirectoryA(255, currentDir))
		ofn.lpstrInitialDir = currentDir;

	ofn.lpstrFilter = filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return FString();
}

FString CEngine::SaveFileDialog(const FString& filter /*= FString()*/)
{
	OPENFILENAMEA ofn;
	CHAR szFile[256] = { 0 };
	CHAR currentDir[256] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;

	ofn.lpstrFilter = filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	ofn.lpstrDefExt = strchr(filter.c_str(), '\0') + 1;

	if (GetSaveFileNameA(&ofn) == TRUE)
		return ofn.lpstrFile;

	return FString();
}

FString CEngine::OpenFolderDialog()
{
	BROWSEINFO bi = { 0 };
	bi.ulFlags = BIF_USENEWUI;
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	CHAR currentDir[256] = { 0 };
	if (GetCurrentDirectoryA(256, currentDir))
		bi.lParam = (LPARAM)currentDir;

	if (pidl != NULL)
	{
		TCHAR path[MAX_PATH];
		if (SHGetPathFromIDList(pidl, path))
		{
			FString sPath = path;
			return sPath;
		}
	}

	return FString();
}

int CEngine::ExecuteProgram(const FString& cmd, bool bWait)
{
	PROCESS_INFORMATION ht{};
	STARTUPINFO si{};
	si.cb = sizeof(si);
	int r = CreateProcessA(NULL, (char*)cmd.c_str(), nullptr, nullptr, false, 0, nullptr, nullptr, &si, &ht);
	if (r != 0)
		return r;

	if (bWait)
	{
		WaitForSingleObject(ht.hProcess, INFINITE);
		
		DWORD ec;
		GetExitCodeProcess(ht.hProcess, &ec);
		r = ec;

		CloseHandle(ht.hProcess);
		CloseHandle(ht.hThread);
	}
	return r;
}

void CEngine::GetMonitorSize(int monitor, int* w, int* h)
{
	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	if (monitor >= count)
		return;
	const GLFWvidmode* mode = glfwGetVideoMode(monitors[monitor]);
	*w = mode->width;
	*h = mode->height;
}

#endif // _WIN32

CGameInstance* CEngine::SetGameInstance(FClass* type)
{
	if (gameInstance)
		gameInstance->Delete();

	gameInstance = (CGameInstance*)CreateObject(type);
	gameInstance->Init();
	return gameInstance;
}

void CEngine::CreatePhysicsApi(FClass* type)
{
	THORIUM_ASSERT(type, "Attempted to create Physics API with invalid type!");

	if (type)
	{
		if (gPhysicsApi)
		{
			// if the already existing api is the same as what we want, we don't need to do anything.
			if (gPhysicsApi->GetClass() == type)
				return;

			gPhysicsApi->Shutdown();
			gPhysicsApi->Delete();
		}
		gPhysicsApi = (IPhysicsApi*)CreateObject(type);
		gPhysicsApi->Init();
		gPhysicsApi->MakeIndestructible();
	}
}
