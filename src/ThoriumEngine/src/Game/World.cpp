
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
#include "Rendering/GraphicsInterface.h"
#include "Assets/TextureAsset.h"
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

	FString gamemodeClass;

	uint32 numSubScenes;
	FSubScene subScenes[numSubScenes];

	uint64 numEnts;
	SerializedEntity entities[numEnts];
*/

void CWorld::AddSubScene(SizeType scene)
{
	if (IsChildWorld())
	{
		CONSOLE_LogError("CWorld", "AddSubScene(), Cannot add a sub scene to a sub scene!");
		return;
	}

	if (!CAssetManager::GetAsset<CScene>(scene))
	{
		CONSOLE_LogError("CWorld", "AddSubScene(), Invalid asset id, asset does not exist!");
		return;
	}

	for (auto& sub : subScenes)
	{
		if (sub.sceneAssetId == scene)
		{
			CONSOLE_LogError("CWorld", "AddSubScene(), Attempted to add already registered sub scene to world!");
			return;
		}
	}

	subScenes.Add({ scene , false, true, nullptr });
}

void CWorld::RemoveSubScene(SizeType scene)
{
	if (IsChildWorld())
		return;

	for (auto it = subScenes.begin(); it != subScenes.end(); it++)
	{
		if (it->sceneAssetId == scene)
			continue;

		if (it->world)
			UnloadSubScene(scene);

		subScenes.Erase(it);
	}
}

void CWorld::LoadSubScene(SizeType scene, bool bAsync)
{
	if (IsChildWorld())
	{
		CONSOLE_LogError("CWorld", "LoadSubScene(), Cannot load a sub scene in a sub scene!");
		return;
	}

	FSubScene* sub = nullptr;
	for (auto& s : subScenes)
		if (s.sceneAssetId == scene)
			sub = &s;

	if (!sub)
		return;

	if (!bAsync)
	{
		CWorld* world = CreateObject<CWorld>();
		world->parent = this;
		world->InitWorld(InitializeInfo());
		world->MakeIndestructible();

		CScene* sceneObj = CAssetManager::GetAsset<CScene>(scene);

		CONSOLE_LogInfo("CEngine", "Loading sub scene '" + sceneObj->Name() + "'");

		world->LoadScene(sceneObj);
		sub->world = world;

		if (bActive)
			world->Start();

		OnSubSceneLoaded.Invoke(*sub);
	}
}

void CWorld::UnloadSubScene(SizeType scene)
{
	if (IsChildWorld())
		return;

	FSubScene* sub = nullptr;
	for (auto& s : subScenes)
		if (s.sceneAssetId == scene)
			sub = &s;
	
	if (!sub)
		return;

	if (sub->world->bActive)
		sub->world->Stop();

	sub->world->Delete();
	subWorlds.Erase(subWorlds.Find(sub->world));

	sub->world = nullptr;
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

	if (sig != CSCENE_SIGNITURE || (version != CSCENE_VERSION && version != CSCENE_VERSION_02 && version != CSCENE_VERSION_03))
	{
		CONSOLE_LogError("CWorld", "Invalid scene file '" + scene->File()->Path() + "'");
		return;
	}

	FString gamemodeType;
	*stream >> gamemodeType;

	sceneSettings.gamemodeClass = gamemodeType;
	if (!sceneSettings.gamemodeClass.Get())
		sceneSettings.gamemodeClass = CGameMode::StaticClass();
	
	if (version > CSCENE_VERSION_03)
	{
		*stream >> &sceneSettings.lightmap;
		*stream >> &sceneSettings.gravity;
	}

	if (version > CSCENE_VERSION_02)
	{
		int numSubScenes;
		*stream >> &numSubScenes;

		subScenes.Resize(numSubScenes);

		for (int i = 0; i < numSubScenes; i++)
		{
			FSubScene& scene = subScenes[i];
			*stream >> &scene;
			scene.world = nullptr;
		}
	}

	SizeType numEnts;
	*stream >> &numEnts;

	//TArray<TPair<CEntity*, FMemStream>> ents;

	for (SizeType i = 0; i < numEnts; i++)
	{
		SizeType entId, numComps;
		FString typeName;
		*stream >> typeName >> &entId >> &numComps;

		FClass* type = CModuleManager::FindClass(typeName);

		CEntity* ent = (CEntity*)CreateEntity(type, "");

		if (!ent)
		{
			CONSOLE_LogError("CWorld", "Serialized entity with unkown type '" + typeName + "'");
			//stream->Seek(dataSize, SEEK_CUR);
			continue;
		}

		entities.erase(ent->entityId);
		entities[entId] = ent;

		ent->entityId = entId;

		for (SizeType i = 0; i < numComps; i++)
		{
			FString typeName;
			FString name;
			SizeType id;
			bool bUserCreated;

			*stream >> typeName >> name >> &id >> &bUserCreated;

			FClass* compClass = CModuleManager::FindClass(typeName);
			if (!compClass)
			{
				CONSOLE_LogWarning("CWorld", "Serialized component has unkown class '" + typeName + "', loading for entity " + ent->Name() + ".");
				continue;
			}

			CEntityComponent* comp = nullptr;
			if (!bUserCreated)
			{
				comp = ent->GetComponent(compClass, name);

				if (comp)
				{
					ent->components.erase(comp->compId);
					ent->components[id] = comp;
				}
			}
			if (!comp)
				comp = ent->AddComponent(compClass, id);

			comp->compId = id;
		}

		/*ents.Add();
		TPair<CEntity*, FMemStream>& d = *ents.last();
		d.Key = ent;
		d.Value.Resize(dataSize);

		stream->Read(d.Value.Data(), dataSize);*/
	}

	for (SizeType i = 0; i < numEnts; i++)
	{
		SizeType entId, dataSize;

		*stream >> &entId >> &dataSize;

		if (auto it = entities.find(entId); it != entities.end())
		{
			FMemStream data;
			data.Resize(dataSize);
			stream->Read(data.Data(), dataSize);

			it->second->Load(data);

			// incase the entity is static we remove it from the dynamic entity list, as it is added by CreateEntity()
			if (it->second->GetType() == ENTITY_STATIC)
				RemoveDynamicEntity(it->second);

#if INCLUDE_EDITOR_DATA
			if (!gIsEditor && it->second->bEditorOnly)
				it->second->Delete();
#endif
		}
		else
			stream->Seek(dataSize, SEEK_CUR);
	}


	if (!bIsTerminal)
		LoadLightData();

	for (auto ent : entities)
		ent.second->PostInit();

	for (auto& subScene : subScenes)
	{
		if (subScene.bLoadOnStart)
			LoadSubScene(subScene.sceneAssetId);
	}
	
	bLoading = false;
	bLoaded = true;

	CONSOLE_LogInfo("CWorld", "Loaded scene with " + FString::ToString(numEnts) + " entities");
}

void CWorld::OnSave(IBaseFStream* stream)
{
	uint sig = CSCENE_SIGNITURE;

	*stream << &sig;

	if (sceneSettings.gamemodeClass.Get())
	{
		*stream << sceneSettings.gamemodeClass.Get()->GetInternalName();
	}
	else
		*stream << FString();

	*stream << &sceneSettings.lightmap << &sceneSettings.gravity;

	uint32 numSubScenes = subScenes.Size();
	*stream << &numSubScenes;

	for (uint32 i = 0; i < numSubScenes; i++)
		*stream << &subScenes[i];

	TMap<SizeType, TObjectPtr<CEntity>>& ents = entities;
	SizeType numEntsOffset = stream->Tell();
	SizeType numEnts = 0;
	*stream << &numEnts;

	for (auto& ent : ents)
	{
		SizeType entId = ent.second->EntityId();
		SizeType numComps = ent.second->GetAllComponents().size();

#if INCLUDE_EDITOR_DATA
		if (ent.second->bEditorEntity)
			continue;
#endif

		* stream << ent.second->GetClass()->GetInternalName() << &entId << &numComps;

		for (auto& comp : ent.second->GetAllComponents())
		{
			*stream << comp.second->GetClass()->GetInternalName();
			*stream << comp.second->Name();

			SizeType id = comp.first;
			*stream << &id;

			bool bUserCreated = comp.second->IsUserCreated();
			*stream << &bUserCreated;
		}
	}

	for (auto& ent : ents)
	{
		SizeType entId = ent.second->EntityId();
		SizeType dataSize;

		FMemStream data;
		ent.second->Serialize(data, FSerializeSettings());

		dataSize = data.Size();
		*stream << &entId << &dataSize;

		stream->Write(data.Data(), data.Size());
		numEnts++;
	}

	stream->Seek(numEntsOffset, SEEK_SET);
	*stream << &numEnts;
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

void CWorld::LoadLightData()
{
	if (!renderScene || !scene)
		return;

	const FString& sceneName = scene->File()->Name();

	FFile* file = CFileSystem::FindFile(scene->File()->Dir()->GetPath() + "/" + sceneName + "_lightdata.bin");
	if (!file)
		return;

	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CWorld", FString("Failed to create file stream for '") + file->Path() + "'");
		return;
	}

	uint32 sig;
	*stream >> &sig;

	uint16 version;
	*stream >> &version;

	uint16 unused;
	*stream >> &unused;

	uint32 atlasCount;
	*stream >> &atlasCount;

	uint32 meshCount;
	*stream >> &meshCount;

	for (uint32 i = 0; i < meshCount; i++)
	{
		SizeType entId, compId;
		*stream >> &entId >> &compId;
		uint32 meshId;
		*stream >> &meshId;

		int32 lightmapId;
		*stream >> &lightmapId;

		FVector2 lightmapOffset, lightmapScale;
		*stream >> &lightmapOffset >> &lightmapScale;

		CEntity* ent = GetEntity(entId);
		if (!ent)
			continue;

		CModelComponent* model = ent->GetComponent<CModelComponent>(compId);
		if (!model)
			continue;

		model->lightmapInfo.Add({ meshId, lightmapId, lightmapOffset, lightmapScale });
	}

	lightmaps.Clear();

	for (uint32 i = 0; i < atlasCount; i++)
	{
		uint32 width, height;
		*stream >> &width >> &height;
		uint32 texelCount;
		*stream >> &texelCount;

		uint16* data = new uint16[texelCount]; // half float format
		stream->Read(data, texelCount * sizeof(uint16));

		// Convert to RGBA format
		uint16* rgbaData = (uint16*)malloc(width * height * 4 * sizeof(uint16));
		for (uint32 j = 0, k = 0; j < texelCount; j += 3, k += 4)
		{
			rgbaData[k] = data[j];			// R
			rgbaData[k + 1] = data[j + 1];	// G
			rgbaData[k + 2] = data[j + 2];	// B
			rgbaData[k + 3] = 0x3c00;		// A (1.0 in half float)
		}

		CTexture* lightmap = new CTexture();
		lightmap->Init(rgbaData, width, height, THTX_FORMAT_RGBA16_FLOAT, THTX_FILTER_LINEAR);
		lightmaps.Add(lightmap);

		delete[] data;
		delete[] rgbaData;
	}
}

void CWorld::Start()
{
	THORIUM_ASSERT(bInitialized, "Cannot start world when world isn't initialized");

	if (!gamemode)
	{
		FClass* gmClass = (scene.IsValid() ? sceneSettings.gamemodeClass.Get() : CGameMode::StaticClass());
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

		OnUpdate.Invoke(dt);
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
#if INCLUDE_EDITOR_DATA
			else if (ent->bEditorEntity)
			{
				ent->Update(dt);
			}
#endif
		}
	}
	//else if (bActive)
	//{
	//	for (auto& ent : dynamicEntities)
	//	{
	//		if (ent->type == ENTITY_DYNAMIC)
	//			ent->Update(dt);
	//	}
	//}

	bInWorldUpdate--;
}

void CWorld::Render()
{
	if (!renderScene)
		return;

	for (auto* c : cameras)
		c->FetchData();

	for (auto* p : primitives)
		p->FetchData();

	for (auto* l : lights)
		l->FetchData();

	for (auto* p : ppVolumes)
		p->FetchData();

	renderScene->SetTime((float)CurTime());

	renderScene->SetPrimitives(primitives);
	renderScene->SetLights(lights);
	renderScene->SetPostProcessVolumes(ppVolumes);

	renderScene->SetCameras(cameras);
	renderScene->SetPrimaryCamera(primaryCamera);

	TArray<ITexture2D*> lightmaps;
	for (auto& lm : this->lightmaps)
		lightmaps.Add((ITexture2D*)lm->GetTextureObject());
	renderScene->SetLightmaps(lightmaps);

	if (subWorlds.Size() > 0)
		RenderSubWorlds();
}

void CWorld::RenderSubWorlds()
{
	for (auto world : subWorlds)
	{
		for (auto* c : world->cameras)
		{
			c->FetchData();
			renderScene->RegisterCamera(c);
		}

		for (auto* p : world->primitives)
		{
			p->FetchData();
			renderScene->RegisterPrimitive(p);
		}

		for (auto* l : world->lights)
		{
			l->FetchData();
			renderScene->RegisterLight(l);
		}

		for (auto* p : world->ppVolumes)
		{
			p->FetchData();
			renderScene->UnregisterPPVolume(p);
		}
	}
}

void CWorld::OnDelete()
{
	for (CWorld* w : subWorlds)
		w->Delete();

	// make a copy of the entities list.
	// on occasions when deleting the entities the iterators would throw an error and this prevents that.
	auto ents = entities;
	for (auto ent = ents.rbegin(); ent != ents.rend(); ent++)
		ent->second->Delete();

	entities.clear();

	delete renderScene;
	renderScene = nullptr;

	delete entityIOManager;

	subWorlds.Clear();

	if (gamemode)
		gamemode->Delete();

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
