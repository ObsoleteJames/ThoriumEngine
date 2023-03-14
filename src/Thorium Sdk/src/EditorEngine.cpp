
#include "EditorEngine.h"
#include "Console.h"
#include "EditorMode.h"

#include <Util/KeyValue.h>
#include "Game/Events.h"
#include "Game/Misc/TransformGizmoEntity.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderScene.h"
#include "Platform/Windows/DirectX/DirectXRenderer.h"

#include "Resources/Material.h"
#include "Resources/ModelAsset.h"
#include "Resources/Scene.h"

void CEditorEngine::Init()
{
	InitMinimal();
	gIsEditor = 1;

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

	gridMesh = new FMesh();
	GenerateGrid(100.0f, 1.f, gridMesh);

	Events::PostLevelChange.Bind(this, &CEditorEngine::OnLevelChange);
	OnObjectSelected.Bind(this, &CEditorEngine::__OnObjectSelected);
}

int CEditorEngine::Run()
{
	CObjectManager::Update();
	Events::OnUpdate.Fire();

	if (!nextSceneName.IsEmpty())
	{
		DoLoadWorld();
		if (bIsPlaying)
			gWorld->Start();
	}

	gWorld->Update(deltaTime);

	if (editorMode)
	{
		if (bIsPlaying)
			editorMode->GameUpdate(deltaTime);
		else
			editorMode->EditorUpdate(deltaTime);
	}

	Events::PostUpdate.Fire();

	{
		FDrawMeshCmd cmd;
		cmd.material = gridMat;
		cmd.mesh = gridMesh;
		cmd.transform = FMatrix(1.f);
		cmd.drawType |= MESH_DRAW_PRIMITIVE_LINES;

		FRenderCommand gridDraw(cmd, R_DEBUG_PASS);
		gWorld->renderScene->PushCommand(gridDraw);
		//worldRenderScene->PushCommand(gridDraw);
	}

	gWorld->Render();

	if (editorMode)
		editorMode->Render();

	return 0;
}

void CEditorEngine::OnExit()
{
	SaveEditorConfig();

	CEngine::OnExit();
}

void CEditorEngine::LoadEditorConfig()
{
	FKeyValue kv(L".project\\config\\Editor.cfg");
	if (kv.IsOpen())
	{
		config.theme = kv.GetValue("theme")->Value;
	}

	if (config.theme.IsEmpty())
		config.theme = "default";
}

void CEditorEngine::SaveEditorConfig()
{
	FKeyValue kv(L".project\\config\\Editor.cfg");
	
	kv.GetValue("theme")->Value = config.theme;

	kv.Save();
}

void CEditorEngine::OnLevelChange()
{
	editorCamera = new CCameraComponent();
	editorCamera->SetWorldPosition({ 0.f, 1.f, -5.f });

	//arrowModel = CreateObject<CModelAsset>();
	//arrowModel->InitFromObj(gArrowObj);

	gWorld->renderScene->SetCamera(editorCamera);

	transformGizmo = gWorld->CreateEntity<CTransformGizmoEntity>();
	transformGizmo->bEditorEntity = true;
	transformGizmo->SetCamera(editorCamera);

	gridMat = CreateObject<CMaterial>();
	gridMat->SetShader("Tools");
	gridMat->SetInt("vType", 1);

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

void CEditorEngine::__OnObjectSelected(const TArray<TObjectPtr<CObject>>& obj)
{
	if (selectedObjects.Size() == 0)
		transformGizmo->SetTargetObject(nullptr);

	TObjectPtr<CEntity> ent = Cast<CEntity>(selectedObjects[0]);
	CSceneComponent* comp;
	if (!ent)
	{
		comp = Cast<CSceneComponent>(selectedObjects[0]);
	}
	else
		comp = ent->RootComponent();

	if (comp)
		transformGizmo->SetTargetObject(comp);
}
