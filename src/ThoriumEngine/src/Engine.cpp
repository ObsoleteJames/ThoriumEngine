
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

#include "ImGui/ImGui.h"

#include <GLFW/glfw3.h>
#include <Util/Assert.h>
#include <Util/KeyValue.h>

#ifdef _WIN32
#include "Platform/Windows/DirectX/DirectXRenderer.h"
#include <windows.h>
#include <shlobj.h>
#endif

#include <filesystem>

#define RENDER_MULTITHREADED 0

CModule& GetModule_Engine();

CEngine* gEngine = nullptr;

bool gIsClient = 0;
bool gIsServer = 0;
bool gIsRunning = 0;
bool gIsEditor = 0;
bool gIsMainGaurded = 0;

static CConCmd cmdLoadScene("scene", [](const TArray<FString>& args) { gEngine->LoadWorld(ToWString(args[0])); });

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

	// Fetch core addons
	FetchAddons(engineMod->Path() + L"\\addons", coreAddons);
	
	bInitialized = true;
}

void CEngine::Init()
{
	InitMinimal();
	THORIUM_ASSERT(LoadProject(), "Failed to load project!");

	CWindow::Init();

	LoadUserConfig();

	gameWindow = new CWindow(userConfig.windowWidth, userConfig.windowHeight, userConfig.windowPosX, userConfig.windowPosY, activeGame.title);
#ifdef _WIN32
	Renderer::CreateRenderer<DirectXRenderer>();
#endif

	gameWindow->swapChain = gRenderer->CreateSwapChain(gameWindow);
	gameWindow->SetWindowMode((CWindow::EWindowMode)userConfig.windowMode);

	// Clear the screen in order to prevent it from being white as the scene loads
	gameWindow->swapChain->GetFrameBuffer()->Clear();
	gameWindow->Present(0, 0);

	inputManager->SetInputWindow(gameWindow);

	InitImGui();

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

		gRenderer->ImGuiBeginFrame();

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

		ImGui::ShowDemoWindow();

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

		gameWindow->swapChain->GetDepthBuffer()->Clear();
		gRenderer->ImGuiRender();

		gameWindow->Present(userConfig.bVSync, 0);

		dtTimer.Stop();
		deltaTime = dtTimer.GetSeconds();

		if (gameWindow->WantsToClose() || bWantsToExit)
			gIsRunning = false;
	}

	gRenderer->ImGuiShutdown();

	OnExit();
	return 0;
}

bool CEngine::LoadProject(const WString& path /*= "."*/)
{
	if (!LoadProjectConfig(path, projectConfig))
		return false;

	CFileSystem::SetCurrentPath(path);
	CConsole::LoadConfig();

	FetchAddons(ToWString(projectConfig.game) + L"\\addons", projectConfig.projectAddons);

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

	// TODO: Make input manager class a config variable.
	inputManager = CreateObject<CInputManager>();
	inputManager->LoadConfig();

	bProjectLoaded = true;
	return true;
}

void CEngine::LoadAddon(FAddon& addon)
{
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

			if (d.instance = (void*)CModuleManager::LoadFLibrary(libName, addon.path + L"\\" + ToWString(d.name)); d.instance == nullptr)
			{
				CONSOLE_LogError("CEngine", "Failed to load addon dependency (FDependency::LIBRARY)\n" + ToFString(addon.path) + "\\" + d.name);
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
		WString libPath = addon.path + L"\\" + ToWString(addon.identity) + L".dll";

		CModule* lib;
		if (int err = CModuleManager::LoadModule(libPath, &lib); err != 0)
		{
			if (err == 1)
				CONSOLE_LogError("CEngine", "Failed to load addon module, file does not exist\n" + ToFString(libPath));
			else
				CONSOLE_LogError("CEngine", "Failed to load addon module, is the library compiled properly?\n" + ToFString(libPath));
		}
		addon.module = lib;
	}

	if (addon.bHasContent)
	{
		addon.mod = CFileSystem::MountMod(addon.path + L"\\content");
		if (!addon.mod)
		{
			CONSOLE_LogError("CEngine", "Failed to mount library content\n" + ToFString(addon.path) + "\\content");
		}
	}
}

void CEngine::Exit()
{
	bWantsToExit = true;
}

void CEngine::OnExit()
{
	SaveUserConfig();
	SaveConsoleLog();

	//delete gWorld;
	gWorld->Delete();
	delete gRenderer;
	delete gameWindow;

	CWindow::Shutdown();

	//for (auto obj : CObjectManager::GetAllObjects())
	//	delete obj.second;
	
	CConsole::Shutdown();
	CResourceManager::Shutdown();
	CModuleManager::Cleanup();
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

	FFile* fontFile = CFileSystem::FindFile(L"fonts\\Roboto-Regular.ttf");

	if (fontFile)
	{
		ImFontConfig cfg;
		//cfg.OversampleH = cfg.OversampleV = 2;
		//cfg.PixelSnapH = true;
		cfg.FontDataOwnedByAtlas = false;

		ImFont* font = io.Fonts->AddFontFromFileTTF(ToFString(fontFile->FullPath()).c_str(), 14, &cfg);
		//font->Scale = 0.6f;
		io.Fonts->AddFontDefault();
	}

	ImGui::StyleColorsDark();

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.46f, 0.98f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.46f, 0.98f, 0.67f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.46f, 0.98f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.42f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.46f, 0.98f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.42f, 0.88f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.21f, 0.26f, 0.38f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.27f, 0.27f, 0.27f, 0.39f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.24f, 0.42f, 0.88f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.46f, 0.98f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.46f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.46f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.42f, 0.88f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.09f, 0.15f, 0.97f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.22f, 0.42f, 1.00f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	//colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.071f, 0.071f, 0.071f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = colors[ImGuiCol_TableBorderStrong];
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
	}

	style.WindowPadding = { 8, 8 };
	style.FramePadding = { 5, 5 };
	style.CellPadding = { 5, 5 };
	style.ItemSpacing = { 8, 8 };
	style.ItemInnerSpacing = { 4, 4 };
	style.WindowRounding = 4;
	style.ChildRounding = 2;
	style.FrameRounding = 3;
	style.GrabRounding = 2;
	style.TabRounding = 2;

	gRenderer->InitImGui(gameWindow);
}

void CEngine::DoLoadWorld()
{
	CScene* pScene = nullptr;
	if (nextSceneName != L"empty")
	{
		pScene = CResourceManager::GetResource<CScene>(nextSceneName);
		if (!pScene)
		{
			CONSOLE_LogError("CEngine", FString("Failed to find scene file '") + ToFString(nextSceneName) + "'");
			return;
		}
	}

	if (gWorld)
		gWorld->Delete();

	gWorld = CreateObject<CWorld>();
	gWorld->MakeIndestructible();

	CONSOLE_LogInfo("CEngine", "Loading scene '" + ToFString(nextSceneName) + "'");

	Events::LevelChange.Invoke();

	if (nextSceneName != L"empty")
	{
		gWorld->LoadScene(pScene);
	}
	else
		gWorld->LoadScene(CreateObject<CScene>());

	gWorld->InitWorld(CWorld::InitializeInfo().RegisterForRendering(false));
	//gWorld->SetRenderScene(worldRenderScene);

	Events::PostLevelChange.Invoke();

	nextSceneName = L"";
}

bool CEngine::LoadProjectConfig(const WString& path, FProject& project)
{
	FKeyValue kv(path + L"\\config\\project.cfg");
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

void CEngine::FetchAddons(const WString& addonFolder, TArray<FAddon>& out)
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
		WString p = addonFolder + L"\\" + entry.path().filename().c_str();

		if (LoadAddonConfig(p, addon))
			out.Add(addon);
	}
}

bool CEngine::LoadAddonConfig(const WString& path, FAddon& out)
{
	FKeyValue cfg(path + L"\\addon.cfg");
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

void CEngine::SaveUserConfig()
{
	FKeyValue kv(ToWString(activeGame.name) + L"\\config\\user.cfg");
	
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

WString CEngine::OSGetDataPath()
{
#ifdef _WIN32
	PWSTR appdata;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &appdata)))
		return WString();

	return WString(appdata);
#endif
}

WString CEngine::OSGetDocumentsPath()
{
#ifdef _WIN32
	PWSTR appdata;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &appdata)))
		return WString();

	return WString(appdata);
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
