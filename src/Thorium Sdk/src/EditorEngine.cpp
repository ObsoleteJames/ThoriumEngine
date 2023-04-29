
#include "EditorEngine.h"
#include "Console.h"
#include "EditorMode.h"

#include <Util/KeyValue.h>
#include "Game/Events.h"
#include "Game/Misc/TransformGizmoEntity.h"
#include "Game/GameInstance.h"
#include "Game/Input/InputManager.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderScene.h"
#include "Misc/Timer.h"
#include "Platform/Windows/DirectX/DirectXRenderer.h"

#include "Resources/Material.h"
#include "Resources/ModelAsset.h"
#include "Resources/Scene.h"

#include <QStandardPaths>

void CEditorEngine::Init()
{
	InitMinimal();
	gIsEditor = true;
	gIsClient = true;

	LoadEditorConfig();

	// Find available themes.
	FDirectory* themesDir = CFileSystem::FindDirectory(L"editor\\themes");
	if (themesDir)
	{
		for (auto* dir : themesDir->GetSubDirectories())
		{
			FFile* themeFile = dir->GetFile(L"theme");

			FKeyValue kv(themeFile->FullPath());
			if (!kv.IsOpen())
				continue;

			FEditorTheme theme;
			theme.name = ToFString(dir->GetName());
			theme.displayName = kv.GetValue("name")->Value;
			themes.Add(theme);
		}
	}

	Renderer::CreateRenderer<DirectXRenderer>();

	for (auto* m : CEditorModeRegistry::Get())
		m->Init();

	if (!gameInstance)
		SetGameInstance<CGameInstance>();

	gridMesh = new FMesh();
	GenerateGrid(100.0f, 1.f, gridMesh);

	TArray<FVertex> boxVerts = {
		{ { -1, 1, 1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ { 1, 1, 1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ { -1, 1, -1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ { 1, 1, -1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
		{ {- 1, -1, 1 }, {}, {}, {}, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
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

	Events::PostLevelChange.Bind(this, &CEditorEngine::OnLevelChange);
	OnObjectSelected.Bind(this, &CEditorEngine::__OnObjectSelected);
}

int CEditorEngine::Run()
{
	inputManager->ClearCache();
	inputManager->BuildInput();

	CResourceManager::Update();
	CObjectManager::Update();
	Events::OnUpdate.Invoke();

	if (!nextSceneName.IsEmpty())
	{
		DoLoadWorld();
		if (bIsPlaying)
			gWorld->Start();
	}

	FTimer updateTimer;

	gWorld->Update(deltaTime);

	if (editorMode)
	{
		if (bIsPlaying)
			editorMode->GameUpdate(deltaTime);
		else
			editorMode->EditorUpdate(deltaTime);
	}

	Events::PostUpdate.Invoke();

	if (!bIsPlaying)
	{
		FDrawMeshCmd cmd;
		cmd.material = gridMat;
		cmd.mesh = gridMesh;
		cmd.transform = FMatrix(1.f);
		cmd.drawType |= MESH_DRAW_PRIMITIVE_LINES;

		FRenderCommand gridDraw(cmd, R_DEBUG_PASS);
		gWorld->renderScene->PushCommand(gridDraw);
		//worldRenderScene->PushCommand(gridDraw);

		DrawSelectionDebug();
	}

	gWorld->Render();

	if (editorMode)
		editorMode->Render();

	updateTimer.Stop();
	updateTime = updateTimer.GetMiliseconds();

	return 0;
}

void CEditorEngine::OnExit()
{
	SaveEditorConfig();

	CEngine::OnExit();
}

void CEditorEngine::LoadEditorConfig()
{
	QString appdataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "\\..\\ThoriumEngine";
	WString configPath = WString((const wchar_t*)appdataPath.data()) + L"\\EditorConfig\\Editor.cfg";
	FKeyValue kv(configPath);
	if (kv.IsOpen())
	{
		config.theme = kv.GetValue("theme")->Value;
	}

	KVCategory* projs = kv.GetCategory("projects", true);
	for (auto& v : projs->GetValues())
	{
		FProject proj;
		proj.name = v.Key;
		proj.dir = ToWString(v.Value.Value);

		RegisterProject(proj);
		//availableProjects.Add(proj);
	}

	if (config.theme.IsEmpty())
		config.theme = "default";
}

void CEditorEngine::SaveEditorConfig()
{
	QString appdataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "\\..\\ThoriumEngine";
	WString configPath = WString((const wchar_t*)appdataPath.data()) + L"\\EditorConfig\\Editor.cfg";
	FKeyValue kv(configPath);
	
	kv.GetValue("theme")->Value = config.theme;

	KVCategory* projs = kv.GetCategory("projects", true);
	for (auto& proj : availableProjects)
		projs->GetValue(proj.name)->Value = ToFString(proj.dir);

	kv.Save();
}

void CEditorEngine::OnLevelChange()
{
	editorCamera = new CCameraProxy();
	editorCamera->position = { 0.f, 1.f, -5.f };

	//arrowModel = CreateObject<CModelAsset>();
	//arrowModel->InitFromObj(gArrowObj);

	gWorld->SetPrimaryCamera(editorCamera);

	transformGizmo = gWorld->CreateEntity<CTransformGizmoEntity>();
	transformGizmo->bEditorEntity = true;
	transformGizmo->SetCamera(editorCamera);

	gridMat = CreateObject<CMaterial>();
	gridMat->SetShader("Tools");
	gridMat->SetInt("vType", 1);

	outlineMat = CreateObject<CMaterial>();
	outlineMat->SetShader("Tools");
	outlineMat->SetInt("vType", 4);
	float col[3] = { 1.f, 0.88f, 0.4f };
	outlineMat->SetVec3("vColorTint", col);

	if (editorMode)
		editorMode->OnLevelChange();
}

void CEditorEngine::GenerateGrid(float gridSize, float quadSize, FMesh*& outMesh)
{
	int numGrids = int(gridSize / quadSize);

	TArray<FVertex> verts;
	//TArray<uint> indices;

	for (int i = 0; i < numGrids + 1; i++)
	{
		float halfGrid = numGrids / 2;

		FVertex a{};
		FVertex b{};

		a.color = { 0.2f, 0.2f, 0.2f };
		b.color = { 0.2f, 0.2f, 0.2f };

		a.position.x = float(i) - halfGrid;
		a.position.z = -halfGrid;
		b.position.x = a.position.x;
		b.position.z = halfGrid;
		
		if (i == (int)halfGrid)
		{
			a.color = { 0.2f, 0.2f, 0.7f };
			b.color = { 0.2f, 0.2f, 0.7f };
		}

		verts.Add(a);
		verts.Add(b);
	}
	for (int i = 0; i < numGrids + 1; i++)
	{
		float halfGrid = numGrids / 2;

		FVertex a{};
		FVertex b{};

		a.color = { 0.2f, 0.2f, 0.2f };
		b.color = { 0.2f, 0.2f, 0.2f };

		a.position.z = float(i) - halfGrid;
		a.position.x = -halfGrid;
		b.position.z = a.position.z;
		b.position.x = halfGrid;

		if (i == (int)halfGrid)
		{
			a.color = { 0.7f, 0.2f, 0.2f };
			b.color = { 0.7f, 0.2f, 0.2f };
		}

		verts.Add(a);
		verts.Add(b);
	}

	//outMesh->numIndices = indices.Size();
	outMesh->numVertices = verts.Size();

	//outMesh->indexBuffer = gRenderer->CreateIndexBuffer(indices);
	outMesh->vertexBuffer = gRenderer->CreateVertexBuffer(verts);
}

void CEditorEngine::SetTheme(const FString& name)
{
	themeIcons.clear();
	config.theme = name;
}

QIcon CEditorEngine::GetIcon(const WString& name)
{
	std::wstring stdName = name.c_str();
	auto it = themeIcons.find(stdName);
	if (it == themeIcons.end())
	{
		FFile* iconFile = CFileSystem::FindFile(L"editor\\themes\\" + ToWString(config.theme) + L"\\icons\\" + name);
		if (!iconFile)
			return QIcon();

		QIcon icon = QIcon(QString((const QChar*)iconFile->FullPath().c_str()));
		themeIcons[stdName] = icon;
		return icon;
	}

	return it->second;
}

QIcon CEditorEngine::GetResourceIcon(const WString& extension)
{
	if (extension.IsEmpty())
		return GetIcon(L"assets\\default.svg");

	WString ext = extension;
	ext.Erase(ext.begin());

	WString iconName = L"assets\\" + ext + L".svg";
	auto it = themeIcons.find(iconName.c_str());
	if (it != themeIcons.end())
		return it->second;

	FFile* iconFile = CFileSystem::FindFile(L"editor\\themes\\" + ToWString(config.theme) + L"\\icons\\" + iconName);
	if (!iconFile)
		return GetIcon(L"assets\\default.svg");

	QIcon icon = QIcon(QString((const QChar*)iconFile->FullPath().c_str()));
	themeIcons[iconName.c_str()] = icon;
	return icon;
}

void CEditorEngine::SetSelectedObject(CObject* obj)
{
	selectedObjects.Clear(); 
	if (obj) 
		selectedObjects.Add(obj); 
	
	OnObjectSelected.Invoke(selectedObjects);
}

void CEditorEngine::SetEditorMode(const FString& modeName)
{
	auto it = CEditorModeRegistry::Get().Find([modeName](CEditorMode* const& mode) { return mode->Name() == modeName; }, 0);
	if (it == CEditorModeRegistry::Get().end())
		return;

	if (editorMode->Name() == modeName)
		return;
	
	if (editorMode)
		editorMode->Close();

	editorMode = *it;
	editorMode->Open();
}

void CEditorEngine::RegisterProject(const FProject& proj)
{
	for (auto& p : availableProjects)
		if (p.name == proj.name || p.dir == proj.dir)
			return;

	availableProjects.Add(proj);
}

void CEditorEngine::DrawSelectionDebug()
{
	if (selectedObjects.Size() == 0)
		return;

	FBounds selectionBounds;

	for (auto obj : selectedObjects)
	{
		if (auto ent = CastChecked<CEntity>(obj); ent)
		{
			selectionBounds = selectionBounds.Combine(ent->GetBounds());
		}
	}

	if (selectionBounds.Size().Magnitude() == 0.f)
		return;

	FDrawMeshCmd cmd;
	cmd.material = outlineMat;
	cmd.mesh = &boxOutlineMesh;
	cmd.transform = FMatrix(1.f).Translate(selectionBounds.position).Scale(selectionBounds.extents);
	cmd.drawType |= MESH_DRAW_PRIMITIVE_LINES;

	FRenderCommand gridDraw(cmd, R_DEBUG_PASS);
	gWorld->renderScene->PushCommand(gridDraw);
}

void CEditorEngine::__OnObjectSelected(const TArray<TObjectPtr<CObject>>& obj)
{
	if (selectedObjects.Size() == 0)
	{
		transformGizmo->SetTargetObject(nullptr);
		return;
	}

	TObjectPtr<CEntity> ent = CastChecked<CEntity>(selectedObjects[0]);
	CSceneComponent* comp;
	if (!ent)
	{
		comp = CastChecked<CSceneComponent>(selectedObjects[0]);
	}
	else
		comp = ent->RootComponent();

	if (comp)
		transformGizmo->SetTargetObject(comp);
}
