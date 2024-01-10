
#include <string>

#include "EditorEngine.h"
#include "Module.h"
#include "Console.h"
#include "Window.h"
#include "Layer.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderScene.h"
#include "Rendering/DebugRenderer.h"
#include "Rendering/Texture.h"
#include "Game/World.h"
#include "Game/Events.h"
#include "Game/GameInstance.h"
#include "Game/Input/InputManager.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Components/ModelComponent.h"
#include "Game/Entities/ModelEntity.h"
#include "Resources/Material.h"
#include "Resources/Scene.h"
#include "FileDialogs.h"
#include "Misc/Timer.h"
#include <Util/KeyValue.h>

#include "AssetBrowserWidget.h"
#include "Layers/PropertyEditor.h"
#include "Layers/ConsoleWidget.h"
#include "Layers/InputOutputWidget.h"
#include "Layers/ProjectSettings.h"
#include "Layers/ObjectDebugger.h"
#include "Layers/MaterialEditor.h"
#include "Layers/AddonsWindow.h"
#include "Layers/ModelEditor.h"
#include "Layers/EditorSettings.h"

#include <map>

#include "Platform/Windows/DirectX/DirectXRenderer.h"
#include "Platform/Windows/DirectX/DirectXFrameBuffer.h"
#include "Platform/Windows/DirectX/DirectXTexture.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"

#include "ThemeManager.h"

#define TEX_VIEW(tex) ((DirectXTexture2D*)tex)->view

void CEditorEngine::Init()
{
	InitMinimal();
	gIsEditor = true;
	gIsClient = true;

	assetBrowser = new CAssetBrowserWidget();
	propertyEditor = AddLayer<CPropertyEditor>();
	consoleWidget = AddLayer<CConsoleWidget>();
	ioWidget = AddLayer<CInputOutputWidget>();
	projSettingsWidget = AddLayer<CProjectSettingsWidget>();
	projSettingsWidget->bEnabled = false;
	addonsWindow = AddLayer<CAddonsWindow>();
	addonsWindow->bEnabled = false;
	editorSettings = AddLayer<CEditorSettingsWidget>();
	editorSettings->bEnabled = false;

	objectDebuggerWidget = AddLayer<CObjectDebugger>();
	objectDebuggerWidget->bEnabled = false;

	LoadEditorConfig();

	CWindow::Init();

	Renderer::CreateRenderer<DirectXRenderer>();

	gameWindow = new CWindow(editorCfg.wndWidth, editorCfg.wndHeight, editorCfg.wndPosX, editorCfg.wndPosY, "Thorium Editor");
	gameWindow->SetSwapChain(gRenderer->CreateSwapChain(gameWindow));
	gameWindow->SetWindowMode((CWindow::EWindowMode)editorCfg.wndMode);

	gameWindow->OnKeyEvent.Bind(this, &CEditorEngine::KeyEventA);
	gameWindow->OnCursorMove.Bind(this, [=](double x, double y) {
		gameWindow->mouseX = x - viewportX;
		gameWindow->mouseY = y - viewportY;
	});

	inputManager = CreateObject<CInputManager>();
	inputManager->SetInputWindow(gameWindow);
	inputManager->SetShowCursor(true);

	//if (!inputManager)
	//{
	//	inputManager = CreateObject<CInputManager>();
	//	inputManager->LoadConfig();
	//}

	//inputManager->SetInputWindow(gameWindow);
	//inputManager->SetShowCursor(true);

	viewportWidth = 1280;
	viewportHeight = 720;
	sceneFrameBuffer = gRenderer->CreateFrameBuffer(1280, 720, TEXTURE_FORMAT_RGBA8_UNORM);
	//sceneDepthBuffer = gRenderer->CreateDepthBuffer({ 1280, 720, TH_DBF_D24_S8, 1, false });

	InitImGui();
	ImGuiIO& io = ImGui::GetIO();
	FString dataPath = ToFString(OSGetDataPath()) + "/ThoriumEngine/EditorConfig/imgui.ini";
	dataPath.ReplaceAll('\\', '/');
	io.IniFilename = (const char*)malloc(dataPath.Size() + 1);
	memcpy((char*)io.IniFilename, dataPath.Data(), dataPath.Size() + 1);
	ImGui::LoadIniSettingsFromDisk(io.IniFilename);

	ThoriumEditor::LoadThemes();
	ThoriumEditor::SetTheme("Default");

	if (!gameInstance)
		SetGameInstance<CGameInstance>();

	editorCamera = new CCameraProxy();
	editorCamera->position = { 0, 1, -1 };
	camController = new CCameraController();
	camController->SetCamera(editorCamera);

	InitEditorData();

	Events::PostLevelChange.Bind(this, &CEditorEngine::OnLevelChange);

	//if (bProjectLoaded)
	//{
	//	assetBrowser->SetDir(activeGame.mod->Name(), FString());
	//	LoadWorld(ToFString(activeGame.startupScene));
	//}

	LoadWorld();
}

int CEditorEngine::Run()
{
	gIsRunning = true;

	deltaTime = 0.02;
	while (gIsRunning)
	{
		FTimer dtTimer;
		
		if (inputManager)
			inputManager->ClearCache();
		CWindow::PollEvents();
		if (inputManager)
			inputManager->BuildInput();

		CResourceManager::Update();
		CObjectManager::Update();

		gRenderer->ImGuiBeginFrame();

		if (!nextSceneName.IsEmpty())
		{
			DoLoadWorld();

			if (bIsPlaying)
				gWorld->Start();
		}

		FTimer updateTimer;

		Events::OnUpdate.Invoke();
		if (!bPaused || bStepFrame)
		{
			gWorld->Update(FMath::Min(deltaTime, 0.25));
			bStepFrame = false;
		}

		Events::PostUpdate.Invoke();

		for (auto* l : removeLayers)
			RemoveLayer(l);
		removeLayers.Clear();

		for (auto& l : layers)
			if (l->bEnabled)
				l->OnUpdate(deltaTime);

		updateTimer.Stop();
		updateTime = updateTimer.GetMiliseconds();

		updateTimer.Begin();

#if RENDER_MULTITHREADED
		gRenderer->JoinRenderThread();
#endif

		// Update scene framebuffer
		{
			int w, h;
			sceneFrameBuffer->GetSize(w, h);
			if (w != viewportWidth || h != viewportHeight)
			{
				sceneFrameBuffer->Resize(viewportWidth, viewportHeight);
				//sceneDepthBuffer->Resize(viewportWidth, viewportHeight);
			}
		}

		UpdateEditor();

		updateTimer.Stop();
		editorUpdateTime = updateTimer.GetMiliseconds();

		updateTimer.Begin();
		gRenderer->BeginRender();

		Events::OnRender.Invoke();

		gWorld->renderScene->SetFrameBuffer(sceneFrameBuffer);
		//gWorld->renderScene->SetDepthBuffer(sceneDepthBuffer);

		if (!bIsPlaying)
		{
			DrawSelectionDebug();
		}

		gWorld->Render();
		gRenderer->PushScene(gWorld->renderScene);

#if RENDER_MULTITHREADED
		gRenderer->RenderMT();
#else
		gRenderer->Render();
#endif
		updateTimer.Stop();
		renderTime = updateTimer.GetMiliseconds();

		updateTimer.Begin();

		gameWindow->swapChain->GetDepthBuffer()->Clear();
		gRenderer->SetFrameBuffer(gameWindow->swapChain->GetFrameBuffer(), gameWindow->swapChain->GetDepthBuffer());
		gRenderer->ImGuiRender();

		updateTimer.Stop();
		imguiRenderTime = updateTimer.GetMiliseconds();

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

void CEditorEngine::OnExit()
{
	SaveEditorConfig();
	SaveConsoleLog();

	delete assetBrowser;
	delete gWorld;
	delete gRenderer;
	delete gameWindow;

	CWindow::Shutdown();

	CConsole::Shutdown();
	CResourceManager::Shutdown();
	CModuleManager::Cleanup();
}

void CEditorEngine::UpdateEditor()
{
	if (bImGuiDemo)
		ImGui::ShowDemoWindow(&bImGuiDemo);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	enum EMenuAction {
		MenuAction_NONE,
		MenuAction_NewScene,
		MenuAction_OpenScene,
		MenuAction_SaveScene,
		MenuAction_SaveSceneAs
	};
	int menuAction = 0;

	SceneFileDialogs();
	
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				menuAction = MenuAction_NewScene;
			if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
				menuAction = MenuAction_OpenScene;
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				menuAction = MenuAction_SaveScene;
			if (ImGui::MenuItem("Save As"))
				menuAction = MenuAction_SaveSceneAs;

			if (!bProjectLoaded)
			{
				ImGui::Separator();

				if (ImGui::MenuItem("Open Project"))
					bOpenProj = true;
			}

			ImGui::Separator();

			ImGui::MenuItem("Build All");
			ImGui::MenuItem("Build Lighting");
			ImGui::MenuItem("Build Cubemaps");
			ImGui::MenuItem("Package Engine Content");

			ImGui::Separator();

			if (ImGui::MenuItem("Quit"))
				Exit();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			ImGui::MenuItem("Undo");
			ImGui::MenuItem("Redo");
			ImGui::MenuItem("Copy");
			ImGui::MenuItem("Paste");

			ImGui::Separator();

			if (ImGui::MenuItem("Generate Build Data"))
				GenerateBuildData();

			ImGui::Separator();

			ImGui::MenuItem("Project Settings", 0, &projSettingsWidget->bEnabled);
			ImGui::MenuItem("Editor Settings", 0, &editorSettings->bEnabled);
			ImGui::MenuItem("Addons", 0, &addonsWindow->bEnabled);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			ImGui::MenuItem("Data Asset Editor");
			if (ImGui::MenuItem("Model Editor"))
				AddLayer<CModelEditor>();
			if (ImGui::MenuItem("Material Editor"))
				AddLayer<CMaterialEditor>();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Scene Outliner", nullptr, &bViewOutliner);
			ImGui::MenuItem("Asset Browser", nullptr, &bViewAssetBrowser);
			ImGui::MenuItem("Properties", nullptr, &propertyEditor->bEnabled);
			ImGui::MenuItem("Console", nullptr, &consoleWidget->bEnabled);

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("ImGui Demo", nullptr, &bImGuiDemo);
			ImGui::MenuItem("Statistics", nullptr, &bViewStats);
			ImGui::MenuItem("Object Debugger", nullptr, &objectDebuggerWidget->bEnabled);

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();

	if (menuAction == MenuAction_SaveScene || (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_S)))
		SaveScene();
	if (menuAction == MenuAction_NewScene || (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_N)))
		NewScene();
	if (menuAction == MenuAction_OpenScene)
		ThoriumEditor::OpenFile("openEditorScene", (FAssetClass*)CScene::StaticClass());

	if (bOpenProj)
		ImGui::OpenPopup("Open Project");

	// Project Selection
	if (ImGui::BeginPopupModal("Open Project", &bOpenProj))
	{
		ImGui::Text("Projects");

		ImGui::BeginChild("projects_list");

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = FMath::Max((int)(panelWidth / 128.f), 1);

		if (ImGui::BeginTable("projects_table", columnCount))
		{
			for (auto& p : availableProjects)
			{
				ImGui::TableNextColumn();

				if (ImGui::Button(("##_project" + p.name).c_str(), ImVec2(112, 112)))
				{
					LoadProject(p.dir);
					// Since the input manager gets reinstantiated, we have to make sure we set it up correctly.
					//inputManager->SetInputWindow(gameWindow);
					//inputManager->SetShowCursor(true);

					assetBrowser->SetDir(activeGame.mod->Name(), FString());

					LoadWorld(ToFString(activeGame.startupScene));
					ImGui::CloseCurrentPopup();
				}
				ImGui::Text(p.displayName.c_str());
			}

			ImGui::EndTable();
		}

		ImGui::EndChild();
		ImGui::EndPopup();
	}

	for (auto& l : layers)
		if (l->bEnabled)
			l->OnUIRender();

	// Viewport
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	if (ImGui::Begin("Scene##_gameSceneViewport"))
	{
		auto wndSize = ImGui::GetContentRegionAvail();
		auto cursorPos = ImGui::GetCursorScreenPos();

		ImGui::PopStyleVar(2);

		if (ImGui::BeginChild("sceneToolBar", ImVec2(wndSize.x, 32)))
		{
			ImGui::SetCursorScreenPos(cursorPos + ImVec2(wndSize.x / 2 - 100, 4));
			ITexture2D* btnPlay = ThoriumEditor::GetThemeIcon("btn-play");
			ITexture2D* btnPause = ThoriumEditor::GetThemeIcon("btn-pause");
			ITexture2D* btnStop = ThoriumEditor::GetThemeIcon("btn-stop");
			ITexture2D* btnStep = ThoriumEditor::GetThemeIcon("btn-stepframe");

			if (ImGui::ImageButton("Play/Pause", (bIsPlaying && !bPaused) ? TEX_VIEW(btnPause) : TEX_VIEW(btnPlay), ImVec2(16, 16)))
			{
				if (!bIsPlaying)
				{
					StartPlay();
					bPaused = false;
				}
				else
					bPaused ^= 1;
			}
			ImGui::SameLine();
			if (ImGui::ImageButton("Stop", TEX_VIEW(btnStop), ImVec2(16, 16)) && bIsPlaying)
			{
				StopPlay();
				bPaused = false;
			}
				//bPaused ^= 1;
			ImGui::SameLine();
			if (ImGui::ImageButton("Step Frame", TEX_VIEW(btnStep), ImVec2(16, 16)))
				bStepFrame = true;
		}
		ImGui::EndChild();

		wndSize = ImGui::GetContentRegionAvail();
		cursorPos = ImGui::GetCursorScreenPos();
		viewportX = cursorPos.x;
		viewportY = cursorPos.y - 24.f;

		DirectXFrameBuffer* fb = (DirectXFrameBuffer*)sceneFrameBuffer;
		ImGui::Image(fb->view, { wndSize.x, wndSize.y });

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* content = ImGui::AcceptDragDropPayload("THORIUM_ASSET_FILE");
			const ImGuiPayload* peek = ImGui::AcceptDragDropPayload("THORIUM_ASSET_FILE", ImGuiDragDropFlags_AcceptPeekOnly);
			if (content)
			{
				FFile* file = *(FFile**)content->Data;
				FAssetClass* type = CResourceManager::GetResourceTypeByFile(file);
				if (type == (FAssetClass*)CScene::StaticClass())
				{
					LoadWorld(file->Path());
				}
				if (type == (FAssetClass*)CMaterial::StaticClass())
				{
					CMaterial* mat = CResourceManager::GetResource<CMaterial>(file->Path());
					DoMaterialDrop(mat, false);
				}
				if (type == (FAssetClass*)CModelAsset::StaticClass())
				{
					CModelAsset* mdl = CResourceManager::GetResource<CModelAsset>(file->Path());
					DoModelAssetDrop(mdl, false);
				}
			}
			if (peek)
			{
				FFile* file = *(FFile**)peek->Data;
				FAssetClass* type = CResourceManager::GetResourceTypeByFile(file);
				if (type == (FAssetClass*)CMaterial::StaticClass())
				{
					CMaterial* mat = CResourceManager::GetResource<CMaterial>(file->Path());
					DoMaterialDrop(mat, true);
				}
			}
			ImGui::EndDragDropTarget();
		}

		camController->Update(deltaTime);
		DoEntRightClick();

		if (ImGui::IsItemClicked() && inputManager && !inputManager->InputEnabled() && bIsPlaying && !bPaused)
			ToggleGameInput();

		if (ImGui::IsItemClicked() && !bIsPlaying)
			DoMousePick();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.75f, 0.75f, 0.75f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 0.3f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 0.25f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);

		ImGui::SetCursorScreenPos(cursorPos + ImVec2(8, 8));

		ImGui::SetNextItemWidth(24);
		ImGui::Button("=##_buttonCameraSettings");

		if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
		{
			ImGui::PopStyleVar();
			ImGui::DragFloat("FOV", &editorCamera->fov, 1.f, 25, 160);
			ImGui::DragFloat("Near Clip", &editorCamera->nearPlane, 0.1f, 0.001f, 10000.f);
			ImGui::DragFloat("Far Clip", &editorCamera->farPlane, 0.1f, 0.001f, 10000.f);

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
			ImGui::EndPopup();
		}

		ImGui::SetNextItemWidth(24);
		ImGui::SameLine();
		ImGui::Button("View##_renderSettings");

		if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
		{
			ImGui::PopStyleVar();
			if (ImGui::MenuItem("Lit"))
				CConsole::Exec("r.materialmode 0");
			if (ImGui::MenuItem("Unlit"))
				CConsole::Exec("r.materialmode 1");
			if (ImGui::MenuItem("Normal"))
				CConsole::Exec("r.materialmode 2");

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
			ImGui::EndPopup();
		}

		ImGui::PopStyleVar();
		ImGui::PopStyleColor(4);

		//ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0.5, 0, 1));
		//ImGui::RenderText(cursorPos + ImGui::GetStyle().FramePadding, "Hello!!");
		//ImGui::PopStyleColor();

		viewportWidth = FMath::Max((int)wndSize.x, 32);
		viewportHeight = FMath::Max((int)wndSize.y, 32);
	}
	else
		ImGui::PopStyleVar(2);
	ImGui::End();

	DoEntityShortcuts();

	// Scene outliner
	if (bViewOutliner)
	{
		if (ImGui::Begin("Scene Outliner##_editorSceneOutliner", &bViewOutliner))
		{
			static FString searchText;
			searchText.Reserve(64);
			ImGui::InputText("Search", searchText.Data(), 63);
			constexpr ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Hideable;

			ImGui::TextColored(ImVec4(1, 1, 0, 1), ("Count Selected: " + FString::ToString(selectedEntities.Size())).c_str());

			ImVec2 region = ImGui::GetContentRegionAvail();
			ImVec2 cursor = ImGui::GetCursorScreenPos();

			ImGui::RenderFrame(cursor, cursor + region, ImGui::ColorConvertFloat4ToU32(ImVec4(0.1f, 0.1f, 0.1f, 1.f)), false);
			
			if (ImGui::BeginTable("outliner_entities", 3, flags, region))
			{
				ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
				ImGui::TableSetupColumn("Type");
				ImGui::TableSetupColumn("Visibility");
				ImGui::TableHeadersRow();

				for (auto& ent : gWorld->GetEntities())
				{
					OutlinerDrawEntity(ent);
				}

				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

	// Asset Browser
	if (bViewAssetBrowser)
	{
		if (ImGui::Begin("Asset Browser##_editorAssetBrowser", &bViewAssetBrowser))
		{
			assetBrowser->RenderUI();
		}
		ImGui::End();
	}

	// Statistics
	if (bViewStats)
	{
		if (ImGui::Begin("Statistics##_editorStats", &bViewStats))
		{
			// Time
			ImGui::Text("frame time: %.2f(ms)", deltaTime * 1000.f);
			ImGui::Text("update: %.2f(ms)", updateTime);
			ImGui::Text("render: %.2f(ms)", renderTime);
			ImGui::Text("editor update: %.2f(ms)", editorUpdateTime);
			if (gWorld)
				ImGui::Text("cur time: %.2f", gWorld->CurTime());

			if (ImGui::TreeNode("Histogram"))
			{
				static bool bPause = false;
				static float values[200] = {};
				static int values_offset = 0;

				if (!bPause)
				{
					values[values_offset] = deltaTime * 1000.f;
					values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
				}

				float highestValue = 0.f;
				for (int i = 0; i < IM_ARRAYSIZE(values); i++)
					if (values[i] > highestValue)
						highestValue = values[i];

				ImGui::Checkbox("Pause", &bPause);
				ImGui::Text("Frame Time");
				ImGui::PlotHistogram("##_frameTimeHistogram", values, IM_ARRAYSIZE(values), values_offset, 0, 0.f, highestValue + 1.f, ImVec2(0, 100));

				static float values2[200] = {};
				static int values2_offset = 0;
				static bool bV2RenderTime = true;

				if (!bPause)
				{
					values2[values2_offset] = !bV2RenderTime ? updateTime : renderTime;
					values2_offset = (values2_offset + 1) % IM_ARRAYSIZE(values2);
				}
				
				//highestValue = 0.f;
				//for (int i = 0; i < IM_ARRAYSIZE(values2); i++)
				//	if (values2[i] > highestValue)
				//		highestValue = values2[i];

				ImGui::Text("Update Time");
				ImGui::SameLine();
				if (ImGui::SmallButton("Update"))
					bV2RenderTime = false;
				ImGui::SameLine();
				if (ImGui::SmallButton("Render"))
					bV2RenderTime = true;

				ImGui::PlotHistogram("##_frameTimeHistogram", values2, IM_ARRAYSIZE(values2), values2_offset, 0, 0.f, highestValue + 1.f, ImVec2(0, 100));

				ImGui::TreePop();
			}

			// Objects
			ImGui::Separator();

			ImGui::Text("object count: %d", CObjectManager::GetAllObjects().size());
			//ImGui::Text("objects to be deleted: %d", CObjectManager::)
			if (gWorld)
				ImGui::Text("entities count: %d", gWorld->GetEntities().Size());

			// Resources
			ImGui::Separator();

			ImGui::Text("Resource Count: %d", CResourceManager::ResourcesCount());
			ImGui::Text("Streaming Resources: %d", CResourceManager::StreamingResourcesCount());
		}
		ImGui::End();
	}
}

bool CEditorEngine::LoadProject(const FString& path)
{
	bool r = CEngine::LoadProject(path);

	bOpenProj = !r;
	return r;
}

void CEditorEngine::LoadEditorConfig()
{
	FKeyValue kv(OSGetDataPath() + "/ThoriumEngine/EditorConfig/Editor.cfg");
	if (!kv.IsOpen())
	{
		CONSOLE_LogWarning("CEditorEngine", "Failed to load editor config, unable to find config file!");
		return;
	}

	editorCfg.wndPosX = kv.GetValue("wndPosX")->AsInt(100);
	editorCfg.wndPosY = kv.GetValue("wndPosY")->AsInt(100);
	editorCfg.wndWidth = kv.GetValue("wndWidth")->AsInt(1920);
	editorCfg.wndHeight = kv.GetValue("wndHeight")->AsInt(1080);
	editorCfg.wndMode = kv.GetValue("wndMode")->AsInt(1);

	KVCategory* projs = kv.GetCategory("projects", true);
	for (auto& v : projs->GetValues())
	{
		FProject proj;
		proj.name = v.Key;
		proj.dir = ToFString(v.Value.Value);

		if (LoadProjectConfig(proj.dir, proj))
			RegisterProject(proj);
	}
}

void CEditorEngine::SaveEditorConfig()
{
	FKeyValue kv(OSGetDataPath() + "/ThoriumEngine/EditorConfig/Editor.cfg");

	gameWindow->UpdateWindowRect();
	gameWindow->GetSize(editorCfg.wndWidth, editorCfg.wndHeight);
	
	kv.SetValue("wndPosX", FString::ToString(gameWindow->WindowedRect.x));
	kv.SetValue("wndPosY", FString::ToString(gameWindow->WindowedRect.y));
	kv.SetValue("wndWidth", FString::ToString(editorCfg.wndWidth));
	kv.SetValue("wndHeight", FString::ToString(editorCfg.wndHeight));
	kv.SetValue("wndMode", FString::ToString((int)gameWindow->GetWindowMode()));

	KVCategory* projs = kv.GetCategory("projects", true);
	for (auto& p : availableProjects)
		projs->SetValue(p.name, ToFString(p.dir));

	kv.Save();
}

void CEditorEngine::GenerateBuildData()
{
	FString cmd = OSGetEnginePath(ENGINE_VERSION) + "/bin/BuildTool.exe \"";
	cmd += CFileSystem::GetCurrentPath() + "/config/project.cfg\" -build ";
#if PLATFORM_WINDOWS
	cmd += "-x64 ";
#endif

#if _DEBUG
	cmd += "-debug";
#elif _DEVELOPMENT
	cmd += "-development";
#elif _RELEASE
	cmd += "-release";
#endif

	ExecuteProgram(cmd);
}

void CEditorEngine::InitEditorData()
{
	TArray<FVertex> boxVerts = {
	{ { -1, 1, 1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ { 1, 1, 1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ { -1, 1, -1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ { 1, 1, -1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ {-1, -1, 1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ { 1, -1, 1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ { -1, -1, -1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ { 1, -1, -1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
	};

	TArray<uint> boxInds = {
		0, 1,
		0, 2,
		2, 3,
		1, 3,

		4, 5,
		4, 6,
		6, 7,
		5, 7,

		0, 4,
		1, 5,
		2, 6,
		3, 7
	};

	boxOutlineMesh.vertexBuffer = gRenderer->CreateVertexBuffer(boxVerts);
	boxOutlineMesh.indexBuffer = gRenderer->CreateIndexBuffer(boxInds);
	boxOutlineMesh.numVertices = boxVerts.Size();
	boxOutlineMesh.numIndices = boxInds.Size();

	boxOutlineMesh.topologyType = FMesh::TOPOLOGY_LINES;

	outlineMat = CreateObject<CMaterial>();
	outlineMat->SetShader("Tools");
	outlineMat->SetInt("vType", 4);
	outlineMat->SetColor("vColorTint", FColor(1.f, 0.88f, 0.4f));
}

void CEditorEngine::NewScene()
{
	//if ()

	LoadWorld();
}

bool CEditorEngine::SaveScene()
{
	if (!gWorld->GetScene()->File())
	{
		ThoriumEditor::SaveFile("saveEditorScene", (FAssetClass*)CScene::StaticClass());
		return false;
	}

	CFStream sdkStream = gWorld->GetScene()->File()->GetSdkStream("wb");
	if (sdkStream.IsOpen())
	{
		FVector camPos = editorCamera->position;
		FQuaternion camRot = editorCamera->rotation;

		sdkStream << &camPos << &camRot;
		sdkStream.Close();
	}

	gWorld->Save();
	return true;
}

void CEditorEngine::StartPlay()
{
	if (!SaveScene())
		return;

	gWorld->Start();
	bIsPlaying = true;
	inputManager->EnableInput();
}

void CEditorEngine::StopPlay()
{
	gWorld->Stop();
	bIsPlaying = false;
	gWorld->SetPrimaryCamera(editorCamera);

	// Reload the scene
	CScene* scene = gWorld->GetScene();
	gWorld->Delete();

	gWorld = CreateObject<CWorld>();
	gWorld->MakeIndestructible();

	Events::LevelChange.Invoke();

	gWorld->LoadScene(scene);
	gWorld->InitWorld(CWorld::InitializeInfo().RegisterForRendering(false));

	Events::PostLevelChange.Invoke();
}

void CEditorEngine::DoMousePick()
{
	if (!gWorld)
		return;

	FRay ray = FRay::MouseToRay(editorCamera, InputManager()->GetMousePos() - FVector2(viewportX, viewportY), { (float)viewportWidth, (float)viewportHeight });
	ray.direction = ray.direction.Normalize();

	auto* scene = gWorld->GetRenderScene();

	FPrimitiveHitInfo hit;

	if (scene->RayCast(ray.origin, ray.direction, &hit))
	{
		CEntity* ent = nullptr;

		TObjectPtr<CObject> obj = hit.hitProxy->GetOwner();
		if (auto comp = CastChecked<CSceneComponent>(obj); comp)
		{
			ent = comp->GetEntity();
		}

		//gDebugRenderer->DrawLine(ray.origin, hit.position, FColor::green, 3, true);
		//gDebugRenderer->DrawLine(ray.origin, ray.origin + ray.direction * 2.f, FColor::red, 3, true);
		//gDebugRenderer->DrawLine(hit.position, hit.position + hit.normal, FColor::navy_blue, 6, true);

		//gDebugRenderer->DrawBox(FTransform(hit.position, FQuaternion(), FVector(0.02f)), FColor::cyan.WithAlpha(0.3f), DebugDrawType_Solid | DebugDrawType_Overlay, 3);

		if (ent)
		{
			if (ImGui::IsKeyDown(ImGuiKey_ModCtrl))
			{
				if (!IsEntitySelected(ent))
					AddSelectedEntity(ent);
				else
					RemoveSelectedEntity(ent);
			}
			else
				SetSelectedEntity(ent);
		}
	}
	else
		SetSelectedEntity(nullptr);
}

void CEditorEngine::DoEntRightClick()
{
	static FVector clickPos;
	if (ImGui::BeginPopup("popupEntViewportContext"))
	{
		if (selectedEntities.Size() > 0)
			EntityContextMenu(selectedEntities[0], clickPos);
		ImGui::EndPopup();
	}

	static ImVec2 mousePos;
	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			mousePos = ImGui::GetIO().MousePos;

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Right) && mousePos.x == ImGui::GetIO().MousePos.x && mousePos.y == ImGui::GetIO().MousePos.y)
		{
			FRay ray = FRay::MouseToRay(editorCamera, InputManager()->GetMousePos() - FVector2(viewportX, viewportY), { (float)viewportWidth, (float)viewportHeight });
			ray.direction = ray.direction.Normalize();

			auto* scene = gWorld->GetRenderScene();

			FPrimitiveHitInfo hit;

			if (scene->RayCast(ray.origin, ray.direction, &hit))
			{
				CEntity* ent = nullptr;

				TObjectPtr<CObject> obj = hit.hitProxy->GetOwner();
				if (auto comp = CastChecked<CSceneComponent>(obj); comp)
				{
					ent = comp->GetEntity();
					SetSelectedEntity(ent);
					ImGui::OpenPopup("popupEntViewportContext");

					clickPos = hit.position;
				}
			}
		}
	}
}

void CEditorEngine::DoMaterialDrop(TObjectPtr<CMaterial> mat, bool bPeek)
{
	if (bPeek)
	{
		//static FPrimitiveHitInfo prevHit{};
		//static CMaterial* prevMat = nullptr;

		//FPrimitiveHitInfo hit;
		//auto* scene = gWorld->GetRenderScene();
		//FRay ray = FRay::MouseToRay(editorCamera, InputManager()->GetMousePos() - FVector2(viewportX, viewportY), { (float)viewportWidth, (float)viewportHeight });
		//ray.direction = ray.direction.Normalize();

		//if (scene->RayCast(ray.origin, ray.direction, &hit))
		//{
		//	if (prevHit.hitProxy && (prevHit.hitProxy != hit.hitProxy || prevHit.materialIndex != hit.materialIndex))
		//	{
		//		TObjectPtr<CObject> obj = prevHit.hitProxy->GetOwner();
		//		if (auto comp = CastChecked<CModelComponent>(obj); comp)
		//		{
		//			comp->SetMaterial(prevMat, hit.materialIndex);
		//		}
		//	}

		//	TObjectPtr<CObject> obj = hit.hitProxy->GetOwner();
		//	if (auto comp = CastChecked<CModelComponent>(obj); comp)
		//	{
		//		if (prevHit.hitProxy != hit.hitProxy || prevHit.materialIndex != hit.materialIndex)
		//			prevMat = comp->GetMaterial(hit.materialIndex);

		//		comp->SetMaterial(mat, hit.materialIndex);
		//	}

		//	prevHit = hit;
		//}
		//else if (prevHit.hitProxy)
		//{
		//	TObjectPtr<CObject> obj = prevHit.hitProxy->GetOwner();
		//	if (auto comp = CastChecked<CModelComponent>(obj); comp)
		//	{
		//		comp->SetMaterial(prevMat, prevHit.materialIndex);
		//	}
		//}
	}
	else
	{
		FPrimitiveHitInfo hit;
		auto* scene = gWorld->GetRenderScene();
		FRay ray = FRay::MouseToRay(editorCamera, InputManager()->GetMousePos() - FVector2(viewportX, viewportY), { (float)viewportWidth, (float)viewportHeight });
		ray.direction = ray.direction.Normalize();

		if (scene->RayCast(ray.origin, ray.direction, &hit))
		{
			TObjectPtr<CObject> obj = hit.hitProxy->GetOwner();
			if (auto comp = CastChecked<CModelComponent>(obj); comp)
			{
				comp->SetMaterial(mat, hit.materialIndex);
			}
		}
	}
}

void CEditorEngine::DoModelAssetDrop(TObjectPtr<CModelAsset> mdl, bool bPeek)
{
	if (bPeek)
	{
		

	}
	else
	{
		FPrimitiveHitInfo hit;
		auto* scene = gWorld->GetRenderScene();
		FRay ray = FRay::MouseToRay(editorCamera, InputManager()->GetMousePos() - FVector2(viewportX, viewportY), { (float)viewportWidth, (float)viewportHeight });
		ray.direction = ray.direction.Normalize();

		if (!scene->RayCast(ray.origin, ray.direction, &hit))
			hit.position = ray.origin + ray.direction;

		CModelEntity* mdlEnt = gWorld->CreateEntity<CModelEntity>(ToFString(mdl->File()->Name()) + " Entity");
		mdlEnt->SetModel(mdl);
		mdlEnt->SetPosition(hit.position);
	}
}

void CEditorEngine::OnLevelChange()
{
	selectedEntities.Clear();
	selectedObject = nullptr;

	if (!bIsPlaying)
	{
		gWorld->SetPrimaryCamera(editorCamera);

		if (gWorld->GetScene()->File())
		{
			CFStream sdkStream = gWorld->GetScene()->File()->GetSdkStream("rb");
			if (sdkStream.IsOpen())
			{
				FVector camPos;
				FQuaternion camRot;

				sdkStream >> &camPos >> &camRot;

				editorCamera->position = camPos;
				editorCamera->rotation = camRot;
				sdkStream.Close();

				// reset the camController's camera so it uses the new oriantation.
				camController->SetCamera(editorCamera);
			}
		}
	}
}

void CEditorEngine::ToggleGameInput()
{
	static bool bShowedCursor = inputManager->CursorVisible();
	if (inputManager->InputEnabled())
		bShowedCursor = inputManager->CursorVisible();
	inputManager->SetInputEnabled(!inputManager->InputEnabled());
	inputManager->SetShowCursor(inputManager->InputEnabled() ? bShowedCursor : true);
}

void CEditorEngine::DrawSelectionDebug()
{
	if (selectedEntities.Size() == 0)
		return;

	for (auto obj : selectedEntities)
	{
		FBounds b = obj->GetBounds();

		if (b.Size().Magnitude() == 0)
			continue;

		FDrawMeshCmd cmd;
		cmd.material = outlineMat;
		cmd.mesh = &boxOutlineMesh;
		cmd.transform = FMatrix(1.f).Translate(b.position).Scale(b.extents);
		cmd.drawType |= MESH_DRAW_PRIMITIVE_LINES;

		FRenderCommand gridDraw(cmd, R_DEBUG_PASS);
		gWorld->renderScene->PushCommand(gridDraw);
	}
}

void CEditorEngine::FocusOnSelection()
{
	if (selectedEntities.Size() == 0)
		return;

	FBounds b;

	for (auto obj : selectedEntities)
		b = b.Combine(obj->GetBounds());

	editorCamera->position = b.position - editorCamera->GetForwardVector() * FMath::Max(b.extents.Magnitude() * 1.5f, 1.f);
}

void CEditorEngine::KeyEventA(EKeyCode key, EInputAction action, EInputMod mod)
{
	if (key == EKeyCode::F1 && mod == IM_SHIFT && action == IE_RELEASE)
	{
		ToggleGameInput();
	}
}

void CEditorEngine::SaveProjectConfig()
{

}

void CEditorEngine::RegisterProject(const FProject& proj)
{
	for (auto& p : availableProjects)
		if (p.name == proj.name || p.dir == proj.dir)
			return;

	availableProjects.Add(proj);
}

bool CEditorEngine::IsEntitySelected(CEntity* ent)
{
	auto it = selectedEntities.Find(ent);
	if (it != selectedEntities.end())
		return true;
	return false;
}

void CEditorEngine::AddSelectedEntity(CEntity* ent)
{
	auto it = selectedEntities.Find(ent);
	if (it != selectedEntities.end())
		return;

	selectedEntities.Add(ent);
	selectedObject = ent;
}

void CEditorEngine::RemoveSelectedEntity(CEntity* ent)
{
	auto it = selectedEntities.Find(ent);
	if (it != selectedEntities.end())
		selectedEntities.Erase(it);

	if (selectedObject == ent && selectedEntities.Size() > 0)
		selectedObject = (CObject*)*selectedEntities.last();
}

void CEditorEngine::SetSelectedEntity(CEntity* ent)
{
	selectedEntities.Clear();
	if (ent)
		selectedEntities.Add(ent);
	selectedObject = ent;
}

void CEditorEngine::AddLayer(TObjectPtr<CLayer> layer)
{
	layers.Add(layer);
	layer->OnAttach();
}

void CEditorEngine::RemoveLayer(TObjectPtr<CLayer> layer)
{
	auto it = layers.Find(layer);
	if (it != layers.end())
	{
		layers.Erase(it);
		layer->OnDetach();
	}
}

void CEditorEngine::PollRemoveLayer(TObjectPtr<CLayer> layer)
{
	removeLayers.Add(layer);
}

void CEditorEngine::OutlinerDrawEntity(CEntity* ent, bool bRoot)
{
	FString type = ent->GetClass()->GetName();

	if (bRoot && CastChecked<CEntity>(ent->GetOwner()))
		return;

	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	const auto& children = ent->GetChildren();
	static TArray<CEntity*> childEnts;
	childEnts.Clear();
	for (auto& c : children)
		if (auto cEnt = CastChecked<CEntity>(c); cEnt)
			childEnts.Add(cEnt);

	int numChildren = childEnts.Size();

	bool bSelected = IsEntitySelected(ent);

	//ImGui::PushStyleVar(ImGuiStyleVar_)
	if (!bSelected)
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
	else
		ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.21f, 0.26f, 0.38f, 1.00f));
	if (ImGui::Selectable(("##ent_select_" + ent->Name()).c_str(), bSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
	{
		if (ImGui::IsKeyDown(ImGuiKey_ModCtrl))
		{
			if (bSelected)
				RemoveSelectedEntity(ent);
			else
				AddSelectedEntity(ent);
		}
		else
			SetSelectedEntity(ent);
	}
	ImGui::PopStyleColor();

	if (ImGui::BeginPopupContextItem())
	{
		EntityContextMenu(ent, FVector());
		ImGui::EndPopup();
	}

	ImGui::SameLine();

	ImVec2 cursor = ImGui::GetCursorScreenPos();

	if (numChildren > 0)
	{
		ImGui::SetNextItemWidth(10);
		ImGui::SetCursorScreenPos(cursor - ImVec2(14, 0));
		bool bOpen = ImGui::TreeNodeEx(("##_tree_" + ent->Name()).c_str(), /*ImGuiTreeNodeFlags_SpanAllColumns |*/ ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);
		ImGui::SameLine();

		ImGui::SetCursorScreenPos(cursor + ImVec2(6, 0));
		ImGui::Text(ent->Name().c_str());
		ImGui::TableNextColumn();
		ImGui::Text(type.c_str());

		if (bOpen)
		{
			for (auto& child : ent->GetChildren())
			{
				auto childEnt = CastChecked<CEntity>(child);
				if (childEnt.IsValid())
					OutlinerDrawEntity(childEnt, false);
			}
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::SetCursorScreenPos(cursor + ImVec2(6, 0));
		ImGui::Text(ent->Name().c_str());
		ImGui::TableNextColumn();
		ImGui::Text(type.c_str());
	}
}

void CEditorEngine::EntityContextMenu(CEntity* ent, const FVector& clickPos)
{
	if (ImGui::MenuItem("Copy", "Ctrl+C"))
		CopyEntity();
	if (ImGui::MenuItem("Paste", "Ctrl+V", nullptr, copyBuffer.dataType == CB_ENTITY))
	{
		if (clickPos.Magnitude() == 0)
			PasteEntity(ent->GetWorldPosition() + FVector((float)FMath::Random(1, 100) / 100.f));
		else
			PasteEntity(clickPos);
	}
	if (ImGui::MenuItem("Duplicate", "Ctrl+D"))
		DupeEntity();

	if (ImGui::MenuItem("Delete", "Ctrl+X"))
	{
		ent->Delete();
		if (IsEntitySelected(ent))
			RemoveSelectedEntity(ent);
	}

	ImGui::Separator();

	if (ImGui::MenuItem("Focus", "F"))
		FocusOnSelection();

	if (ImGui::MenuItem("Make Hidden", "H"))
		ent->bIsVisible = false;

	if (ImGui::MenuItem("Make Visible", "H"))
		ent->bIsVisible = true;

	if (ImGui::BeginMenu("Attach To"))
	{
		for (auto& e : gWorld->GetEntities())
		{
			if (e == ent)
				continue;

			if (ImGui::MenuItem((e->Name() + "##_attachToEnt_" + FString::ToString((SizeType)&*e)).c_str()))
				ent->RootComponent()->AttachTo(e->RootComponent());
		}

		ImGui::EndMenu();
	}
}

void CEditorEngine::DoEntityShortcuts()
{
	if (selectedEntities.Size() == 0)
		return;

	if (ImGui::IsKeyReleased(ImGuiKey_F))
		FocusOnSelection();

	if (ImGui::IsKeyReleased(ImGuiKey_H))
	{
		bool bVis = selectedEntities[0]->bIsVisible;
		for (auto& obj : selectedEntities)
		{
			obj->bIsVisible = !bVis;
		}
	}

	if (ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyReleased(ImGuiKey_X))
	{
		for (auto& ent : selectedEntities)
		{
			ent->Delete();
			if (IsEntitySelected(ent))
				RemoveSelectedEntity(ent);
		}
	}
}

void CEditorEngine::CopyEntity()
{
	if (selectedEntities.Size() == 0)
		return;

	copyBuffer.dataType = CB_ENTITY;
	copyBuffer.data = FMemStream();
	copyBuffer.type = selectedEntities[0]->GetClass();

	selectedEntities[0]->Serialize(copyBuffer.data);
}

void CEditorEngine::PasteEntity(const FVector& pos)
{
	if (copyBuffer.dataType == CB_NONE)
		return;

	CEntity* ent = gWorld->CreateEntity(copyBuffer.type, FString());

	copyBuffer.data.Seek(SEEK_SET, 0);
	ent->Load(copyBuffer.data);
	ent->SetWorldPosition(pos);

	SetSelectedEntity(ent);
}

void CEditorEngine::SceneFileDialogs()
{
	FString f;
	FString m;
	if (ThoriumEditor::AcceptFile("saveEditorScene", &f, &m) && !f.IsEmpty())
	{
		CResourceManager::RegisterNewResource(gWorld->GetScene(), f, m);
		gWorld->Save();
	}
	if (ThoriumEditor::AcceptFile("openEditorScene", &f, &m) && !f.IsEmpty())
	{
		LoadWorld(f);
	}
}

void CEditorEngine::DupeEntity()
{

}
