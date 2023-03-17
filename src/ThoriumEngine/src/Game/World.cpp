
#include <string>

#include "World.h"
#include "Engine.h"
#include "Game/Events.h"
#include "Game/GameInstance.h"
#include "Game/Components/ModelComponent.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"
#include "Rendering/Renderer.h"
#include "Resources/Scene.h"
#include "Console.h"
#include <Util/Assert.h>
#include <Util/FStream.h>

static bool bLoadingScene = false;
static int bInWorldUpdate = 0;

CWorld* gWorld = nullptr;

/*
	- Scene Binary format -

	uint32 sig;
	uint8 version;

	// add some scene settings here.

	uint64 numEnts;
	SerializedEntity entities[numEnts];
*/

void CWorld::SetNewWorld(CWorld* world)
{
	if (bInWorldUpdate > 0)
		return;

	if (gWorld)
		gWorld->Delete();

	gWorld = world;
}

bool CWorld::IsInUpdate()
{
	return bInWorldUpdate;
}

void CWorld::InitWorld(const InitializeInfo& initInfo)
{
	if (initInfo.bCreateRenderScene)
	{
		renderScene = new CRenderScene();
	}

	if (initInfo.bRegisterForRendering)
	{
		Events::OnRender.Bind([=]() { this->Render(); gRenderer->PushScene(renderScene); });
	}


	bInitialized = true;
}

void CWorld::StreamScene(CScene* scene)
{
	if (bLoaded)
		return;
}

void CWorld::LoadScene(CScene* ptr)
{
	if (bLoaded)
		return;

	scene = ptr;
	if (!scene->File())
		return;

	TUniquePtr<IBaseFStream> stream = scene->File()->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError(FString("Failed to create file stream for '") + ToFString(scene->File()->Path()) + "'");
		return;
	}

	uint sig;
	uint8 version;

	*stream >> &sig >> &version;

	if (sig != CSCENE_SIGNITURE || version != CSCENE_VERSION)
	{
		CONSOLE_LogError("Invalid scene file '" + ToFString(scene->File()->Path()) + "'");
		return;
	}

	FString gamemodeType;
	*stream >> gamemodeType;

	scene->gamemodeClass = gamemodeType;
	if (!scene->gamemodeClass.Get())
		scene->gamemodeClass = CGameMode::StaticClass();

	SizeType numEnts;
	*stream >> &numEnts;

	TArray<TPair<CEntity*, FMemStream>> ents;

	for (SizeType i = 0; i < numEnts; i++)
	{
		SizeType entId, dataSize;
		FString typeName;
		*stream >> typeName >> &entId >> &dataSize;

		FClass* type = CModuleManager::FindClass(typeName);

		CEntity* ent = (CEntity*)CreateEntity(type, "");

		if (!ent)
		{
			CONSOLE_LogError("Serialized entity with invalid type '" + typeName + "'");
			stream->Seek(dataSize, SEEK_CUR);
			continue;
		}

		ent->SetId(entId);

		ents.Add();
		TPair<CEntity*, FMemStream>& d = *ents.last();
		d.Key = ent;
		d.Value.Resize(dataSize);

		stream->Read(d.Value.Data(), dataSize);
	}

	for (auto& d : ents)
	{
		d.Key->Load(d.Value);

		if (!gIsEditor && d.Key->bEditorOnly)
			d.Key->Delete();
	}

	bLoaded = true;
}

void CWorld::Save()
{
	scene->Save(this);
}

CEntity* CWorld::CreateEntity(FClass* classType, const FString& name)
{
	CEntity* r = (CEntity*)CreateObject(classType, name);
	if (!r)
		return nullptr;

	r->world = this;
	r->Init();

	if (bActive)
		r->OnStart();

	OnEntityCreated.Fire(r);

	entities.Add(r);
	return r;
}

void CWorld::SetGameMode(const TObjectPtr<CGameMode>& gm)
{
	if (bActive)
	{
		CONSOLE_LogWarning("Cannot set gamemode while world is active!");
		return;
	}

	if (!gm)
		return;

	if (gamemode)
		gamemode->Delete();

	gamemode = gm;
	gamemode->Init();
}

void CWorld::Start()
{
	THORIUM_ASSERT(bInitialized, "Cannot start world when world isn't initialized");

	if (!gamemode)
	{
		FClass* gmClass = (scene.IsValid() ? scene->gamemodeClass.Get() : CGameMode::StaticClass());
		gamemode = (CGameMode*)CreateObject(gmClass, FString());
		gamemode->Init();
	}

	gEngine->GameInstance()->OnStart();
	gamemode->OnStart();

	gEngine->GameInstance()->SpawnLocalPlayers();

	for (auto* sub : subWorlds)
		sub->Start();

	for (auto& ent : entities)
		ent->OnStart();

	bActive = true;
	time = 0.f;
}

void CWorld::Stop()
{
	if (!bActive)
		return;

	for (auto& ent : entities)
		ent->OnStop();

	for (auto& sub : subWorlds)
		sub->Stop();

	gEngine->GameInstance()->OnStop();
	gamemode->Delete();
	gamemode = nullptr;

	bActive = false;
	time = 0.f;
}

void CWorld::Update(double dt)
{
	bInWorldUpdate++;
	time += dt;

	if (bActive)
		for (auto* world : subWorlds)
			world->Update(dt);

	if (gIsEditor)
	{
		for (auto& ent : entities)
		{
			if (bActive)
			{
				if (ent->type == ENTITY_DYNAMIC && !ent->bEditorEntity)
					ent->Update(dt);
			}
			else if (ent->bEditorEntity)
			{
				ent->Update(dt);
			}
		}
	}
	else if (bActive)
	{
		for (auto& ent : entities)
		{
			if (ent->type == ENTITY_DYNAMIC)
				ent->Update(dt);
		}
	}

	bInWorldUpdate--;
}

void CWorld::Render()
{
	if (!renderScene)
		return;

	renderScene->SetTime((float)CurTime());

	for (auto* p : primitives)
	{
		p->FetchData();
	}

	for (auto* l : lights)
		l->FetchData();

	renderScene->SetPrimitives(primitives);
	renderScene->SetLights(lights);

	renderScene->SetCameras(cameras);
	renderScene->SetPrimaryCamera(primaryCamera);

	//for (CModelComponent* mdl : models)
	//{
	//	CEntity* ent = mdl->GetEntity();
	//	
	//	// TODO: Check if this is visible from ownership.
	//	if (!ent || !ent->bIsVisible || !mdl->bIsVisible || !mdl->GetModel())
	//		continue;

	//	float distanceFromCamera = FVector::Distance(renderScene->GetCamera()->GetWorldPosition(), mdl->GetWorldPosition());
	//	int lodLevel = mdl->GetModel()->GetLodFromDistance(distanceFromCamera);
	//	mdl->GetModel()->Load(lodLevel);

	//	FMatrix modelMatrix = FMatrix(1.f).Translate(mdl->GetWorldPosition()).Scale(mdl->GetWorldScale()) * mdl->GetWorldRotation();
	//	//modelMatrix = modelMatrix.Scale(mdl->GetWorldScale()).Translate(mdl->GetWorldPosition());

	//	TArray<FMesh*> meshes = mdl->GetVisibleMeshes(lodLevel);
	//	for (auto* mesh : meshes)
	//	{
	//		FDrawMeshCmd cmd;
	//		CMaterial* mat = mdl->GetMaterial(mesh->materialIndex);
	//		if (!mat)
	//			continue;

	//		cmd.transform = modelMatrix;
	//		cmd.mesh = mesh;
	//		cmd.material = mat;
	//		
	//		if (mesh->topologyType != FMesh::TOPOLOGY_TRIANGLES)
	//			cmd.drawType |= (mesh->topologyType == FMesh::TOPOLOGY_LINES ? MESH_DRAW_PRIMITIVE_LINES : MESH_DRAW_PRIMITIVE_POINTS);

	//		ERenderPass rp{};
	//		uint8 matType = mat->GetShaderSource()->type;
	//		if (matType == CShaderSource::ST_DEFERRED)
	//			rp = R_DEFERRED_PASS;
	//		else if (matType == CShaderSource::ST_FORWARD)
	//			rp = mat->GetAlpha() < 1.f ? R_TRANSPARENT_PASS : R_FORWARD_PASS;

	//		renderScene->PushCommand(FRenderCommand(cmd, rp));
	//	}
	//}
}

void CWorld::OnDelete()
{
	for (CWorld* w : subWorlds)
		w->Delete();

	delete renderScene;

	subWorlds.Clear();

	if (gamemode)
		gamemode->Delete();

	for (CEntity* ent : entities)
		ent->Delete();

	entities.Clear();
}
