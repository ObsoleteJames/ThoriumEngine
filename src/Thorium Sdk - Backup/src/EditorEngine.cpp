
#include "EditorEngine.h"
#include "Console.h"

#include "Game/Events.h"
#include "Resources/Material.h"
#include "Resources/ModelAsset.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderScene.h"
#include "Platform/Windows/DirectX/DirectXRenderer.h"

void CEditorEngine::Init()
{
	InitMinimal();
	gIsEditor = 1;

	Renderer::CreateRenderer<DirectXRenderer>();

	gWorld = new CWorld();
	gWorld->InitWorld(CWorld::InitializeInfo().RegisterForRendering(false).CreateRenderScene(false));

	editorCamera = new CCameraComponent();
	editorCamera->SetWorldPosition({ 0.f, 1.f, -5.f });

	toolsMat = new CMaterial("Tools");

	TArray<FVertex> planeVerts = {
		{{ -0.5f, 0,  0.5f }},
		{{ -0.5f, 0, -0.5f }},
		{{  0.5f, 0, -0.5f }},
		{{  0.5f, 0,  0.5f }}
	};
	TArray<uint> planeIndices = { 1, 0, 2, 0, 3, 2 };

	planeMesh = new FMesh();
	planeMesh->numIndices = 6;
	planeMesh->numVertices = 4;

	planeMesh->vertexBuffer = gRenderer->CreateVertexBuffer(planeVerts);
	planeMesh->indexBuffer = gRenderer->CreateIndexBuffer(planeIndices);
}

int CEditorEngine::Run()
{
	CObjectManager::Update();
	Events::OnUpdate.Fire();

	gWorld->Update(deltaTime);
	
	Events::PostUpdate.Fire();

	gRenderer->BeginRender();
	gWorld->Render();

	{
		toolsMat->SetInt("Draw Type", 1);
		toolsMat->SetFloat("Variable1", 1.f);

		FDrawMeshCmd cmd;
		cmd.material = toolsMat;
		cmd.mesh = planeMesh;
		cmd.transform = FMatrix(1.f).Scale({ 100.f, 1.f, 100.f });

		FRenderCommand gridDraw(cmd, R_FORWARD_PASS);
		worldRenderScene->PushCommand(gridDraw);
	}

	return 0;
}
