
#include <string>

#include "World.h"
#include "Engine.h"
#include "Game/Entity.h"
#include "Game/Events.h"
#include "Game/GameInstance.h"
#include "Game/Components/ModelComponent.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"
#include "Rendering/PostProcessing.h"
#include "Rendering/Renderer.h"
#include "Assets/Scene.h"
#include "Physics/PhysicsWorld.h"
#include "Console.h"
#include "Object/ObjectHandle.h"
#include <Util/Assert.h>
#include <Util/FStream.h>

static bool bLoadingScene = false;
static int bInWorldUpdate = 0;

CWorld* gWorld = nullptr;

CWorld::CWorld() : bInitialized(0), bActive(0), time(0.f), bLoaded(false), bLoading(false), bStreaming(false)
{
	bIndestructible = true;
}

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

void CWorld::InitWorld(const InitializeInfo& i)
{
	initInfo = i;
	if (!parent)
	{
		if (initInfo.bCreateRenderScene && gRenderer)
			renderScene = new CRenderScene();

		if (initInfo.bRegisterForRendering && gRenderer)
			Events::OnRender.Bind(this, [=]() { this->Render(); gRenderer->PushScene(renderScene); });

		if (initInfo.bCreatePhyiscsWorld && gPhysicsApi)
			physicsWorld = gPhysicsApi->CreateWorld();

		entityIOManager = new CEntityIOManager(this);
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

	bLoading = true;

	scene = ptr;
	if (!scene->File())
		return;

	TUniquePtr<IBaseFStream> stream = scene->File()->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CWorld", FString("Failed to create file stream for '") + scene->File()->Path() + "'");
		return;
	}

	stream->Seek(sizeof(FAssetHeader), SEEK_SET);

	uint sig;
	uint8 version = scene->Version();

	*stream >> &sig;

	if (sig != CSCENE_SIGNITURE || version != CSCENE_VERSION)
	{
		CONSOLE_LogError("CWorld", "Invalid scene file '" + scene->File()->Path() + "'");
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
			CONSOLE_LogError("CWorld", "Serialized entity with unkown type '" + typeName + "'");
			stream->Seek(dataSize, SEEK_CUR);
			continue;
		}

		entities.erase(ent->entityId);
		entities[entId] = ent;

		ent->entityId = entId;

		ents.Add();
		TPair<CEntity*, FMemStream>& d = *ents.last();
		d.Key = ent;
		d.Value.Resize(dataSize);

		stream->Read(d.Value.Data(), dataSize);
	}

	for (auto& d : ents)
	{
		d.Key->Load(d.Value);

		// incase the entity is static we remove it from the dynamic entity list, as it is added by CreateEntity()
		if (d.Key->GetType() == ENTITY_STATIC)
			RemoveDynamicEntity(d.Key);

		if (!gIsEditor && d.Key->bEditorOnly)
			d.Key->Delete();
	}

	for (auto ent : entities)
		ent.second->PostInit();
	
	bLoading = false;
	bLoaded = true;

	CONSOLE_LogInfo("CWorld", "Loaded scene with " + FString::ToString(numEnts) + " entities");
}

void CWorld::Save()
{
	scene->world = this;
	scene->Save();
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

	r->MakeDynamic();

	auto findE = entities.find(r->EntityId());
	while (findE != entities.end())
	{
		// re-generate the id
		r->entityId = FGuid();

		findE = entities.find(r->EntityId());
	}
	entities[r->EntityId()] = r;

	OnEntityCreated.Invoke(r);
	if (!bLoading)
		r->PostInit();

	return r;
}

CEntity* CWorld::GetEntity(const FString& name)
{
	for (auto& ent : entities)
		if (ent.second->Name() == name)
			return ent.second;

	return nullptr;
}

CEntity* CWorld::GetEntity(SizeType entityId)
{
	for (auto& ent : entities)
		if (ent.first == entityId)
			return ent.second;

	return nullptr;
}

void CWorld::SetGameMode(const TObjectPtr<CGameMode>& gm)
{
	if (bActive)
	{
		CONSOLE_LogWarning("CWorld", "Cannot set gamemode while world is active!");
		return;
	}

	if (!gm)
		return;

	if (gamemode)
		gamemode->Delete();

	gamemode = gm;
	gamemode->world = this;
	gamemode->Init();
}

void CWorld::Start()
{
	THORIUM_ASSERT(bInitialized, "Cannot start world when world isn't initialized");

	if (!gamemode)
	{
		FClass* gmClass = (scene.IsValid() ? scene->gamemodeClass.Get() : CGameMode::StaticClass());
		auto* gm = (CGameMode*)CreateObject(gmClass, FString());
		SetGameMode(gm);
	}

	bActive = true;
	time = 0.f;

	gEngine->GameInstance()->Start();
	gamemode->OnStart();

	gEngine->GameInstance()->SpawnLocalPlayers();

	for (auto* sub : subWorlds)
		sub->Start();

	for (auto& ent : entities)
		ent.second->OnStart();

	if (physicsWorld)
		physicsWorld->Start();
}

void CWorld::Stop()
{
	if (!bActive)
		return;

	for (auto& ent : entities)
		ent.second->OnStop();

	for (auto& sub : subWorlds)
		sub->Stop();

	gEngine->GameInstance()->Stop();
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
	{
		gamemode->Update(dt);

		for (auto* world : subWorlds)
			world->Update(dt);

		if (entityIOManager)
			entityIOManager->Update();

		if (physicsWorld)
		{
			physicsWorld->Update(dt);
			physicsWorld->ResolveCollisions();
		}
	}

	if (gIsEditor)
	{
		for (auto& ent : dynamicEntities)
		{
			if (bActive)
			{
				if (ent->type == ENTITY_DYNAMIC)
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
		for (auto& ent : dynamicEntities)
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

	for (auto* c : cameras)
		c->FetchData();

	for (auto* p : primitives)
		p->FetchData();

	for (auto* l : lights)
		l->FetchData();

	for (auto* p : ppVolumes)
		p->FetchData();

	renderScene->SetPrimitives(primitives);
	renderScene->SetLights(lights);
	renderScene->SetPostProcessVolumes(ppVolumes);

	renderScene->SetCameras(cameras);
	renderScene->SetPrimaryCamera(primaryCamera);
}

void CWorld::OnDelete()
{
	for (CWorld* w : subWorlds)
		w->Delete();

	delete renderScene;
	renderScene = nullptr;

	delete entityIOManager;

	subWorlds.Clear();

	if (gamemode)
		gamemode->Delete();

	// make a copy of the entities list.
	// on occasions when deleting the entities the iterators would throw an error and this prevents that.
	auto ents = entities;
	for (auto ent = ents.rbegin(); ent != ents.rend(); ent++)
		ent->second->Delete();

	entities.clear();
	
	if (physicsWorld)
	{
		gPhysicsApi->DestroyWorld(physicsWorld);
		physicsWorld = nullptr;
	}

	if (initInfo.bRegisterForRendering)
		Events::OnRender.RemoveAll(this);
}

void CWorld::RemoveEntity(CEntity* ent)
{
	auto it = entities.find(ent->entityId);
	if (it != entities.end())
	{
		if (bActive)
			ent->OnStop();

		OnEntityDeleted.Invoke(ent);
		entities.erase(ent->entityId);
	}
}

CEntityIOManager* CWorld::GetEntityIOManager() const
{
	if (parent) 
		return parent->GetEntityIOManager(); 
	return entityIOManager;
}

void CWorld::RegisterDynamicEntity(CEntity* ent)
{
	auto it = dynamicEntities.Find(ent);
	if (it == dynamicEntities.end())
		dynamicEntities.Add(ent);
}

void CWorld::RemoveDynamicEntity(CEntity* ent)
{
	auto it = dynamicEntities.Find(ent);
	if (it != dynamicEntities.end())
		dynamicEntities.Erase(it);
}

void FWorldRegisterer::UnregisterEntity(CWorld* world, CEntity* ent)
{
	if (world)
		world->RemoveEntity(ent);
}

CEntityIOManager::CEntityIOManager(CWorld* w) : world(w)
{
}

void CEntityIOManager::Update()
{
	for (auto it = delayedEvents.rbegin(); it != delayedEvents.rend(); it++)
	{
		if (world->CurTime() > it->time)
		{
			_Fire(&*it);
			delayedEvents.Erase(it);
		}
	}
}

void CEntityIOManager::FireEvent(CEntity* caller, SizeType outputIndex)
{
	const FOutputBinding& binding = caller->GetOutput(outputIndex);

	FEntityOutputEvent e = { caller, outputIndex, binding.delay + (float)world->CurTime() };
	if (binding.delay > 0.f)
	{
		delayedEvents.Add(e);
		return;
	}

	_Fire(&e);
}

void CEntityIOManager::_Fire(FEntityOutputEvent* event)
{
	if (!curInstigator)
		curInstigator = event->caller;

	callerStack.Add(event->caller);

	const FOutputBinding& binding = event->caller->GetOutput(event->outputIndex);

	CEntity* target = binding.targetObject.GetAs<CEntity>();
	const FFunction* func = nullptr;
	FStack funcStack(FMath::Max((uint32)binding.arguments.Size(), 1u));

	if (!target)
	{
		CONSOLE_LogError("CEntityIOManager", "Entity output has invalid target object! Entity: " + event->caller->Name() + "  Output Index: " + FString::ToString(event->outputIndex));
		goto exit;
	}

	for (FClass* c = target->GetClass(); c != nullptr; c = c->GetBaseClass())
	{
		for (const FFunction* f = c->GetFunctionList(); f != nullptr; f = f->next)
		{
			if (f->name == binding.targetInput)
			{
				func = f;
				goto foundFunc;
			}
		}
	}
foundFunc:

	if (binding.arguments.Size() > 0)
		funcStack.Push((void*)binding.arguments.Data(), binding.arguments.Size());

	if (!func)
	{
		CONSOLE_LogError("CEntityIOManager", "Entity output has invalid input! Entity: " + event->caller->Name() + "  Output Index: " + FString::ToString(event->outputIndex));
		goto exit;
	}

	func->execFunc(target, funcStack);

	((FOutputBinding*)&binding)->fireCount++;

exit:
	callerStack.PopBack();
	if (callerStack.Size() == 0)
		curInstigator = nullptr;
}
