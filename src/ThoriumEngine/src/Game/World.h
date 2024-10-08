#pragma once

#include <Util/Core.h>
#include <Util/Event.h>
#include "EngineCore.h"
#include "Object/Delegate.h"
#include "Math/Vectors.h"
#include <mutex>

#include "World.generated.h"

class CWorld;
class CScene;
class IBaseWindow;
class CPrimitiveProxy;
class CLightProxy;
class CCameraProxy;
class CPostProcessVolumeProxy;
class CRenderScene;
class CGameMode;
class CEntity;
class IPhysicsWorld;

extern ENGINE_API CWorld* gWorld;

enum ENetMode
{
	NetMode_Singeplayer, // A Standalone game with no networking.
	NetMode_Dedicated, // A Dedicated server which clients can join.
	NetMode_Host, // A Client running a server which other clients can join.
	NetMode_Client // A Client connected to a server.
};

struct FEntityOutputEvent
{
public:
	TObjectPtr<CEntity> caller;
	SizeType outputIndex;

	// activation time
	float time;
};

class CEntityIOManager
{
public:
	CEntityIOManager(CWorld* world);

	void Update();

	void FireEvent(CEntity* caller, SizeType outputIndex);

	inline CEntity* GetInstigator() const { return curInstigator; }
	inline CEntity* GetCaller() const { if (callerStack.Size() > 0) return *callerStack.last(); return nullptr; }

private:
	void _Fire(FEntityOutputEvent* event);

private:
	CWorld* world;

	CEntity* curInstigator;
	TArray<CEntity*> callerStack;

	TArray<FEntityOutputEvent> delayedEvents;
};

/*
 *	World Class
 */
CLASS()
class ENGINE_API CWorld : public CObject
{
	GENERATED_BODY()

	friend class CEntity;
	friend class CEngine;
	friend class CScene;
	friend class CEditorEngine;
	friend class FWorldRegisterer;

public:
	struct InitializeInfo
	{
		InitializeInfo() : bCreatePhyiscsWorld(1), bCreateAISystems(1), bCreateRenderScene(1), bRegisterForRendering(1) {}

		bool bCreatePhyiscsWorld : 1;
		bool bCreateAISystems : 1;
		bool bCreateRenderScene : 1;
		bool bRegisterForRendering : 1;

		inline InitializeInfo& CreatePhyiscsWorld(bool b) { bCreatePhyiscsWorld = b; return *this; }
		inline InitializeInfo& CreateAISystems(bool b) { bCreateAISystems = b; return *this; }
		inline InitializeInfo& CreateRenderScene(bool b) { bCreateRenderScene = b; return *this; }
		inline InitializeInfo& RegisterForRendering(bool b) { bRegisterForRendering = b; return *this; }
	};

public:
	CWorld();
	
	//static FSceneLoadResult LoadScene(const FString& name);
	//static void LoadSceneAsync(const FString& name, std::function<void(FSceneLoadResult)> onComplete);

	//static FSceneLoadResult LoadSubScene(const FString& name);
	//static void LoadSubSceneAsync(const FString& name, std::function<void(FSceneLoadResult)> onComplete);

	static void SetNewWorld(CWorld* world);

	static bool IsInUpdate();

public:
	void InitWorld(const CWorld::InitializeInfo& initInfo);
	void StreamScene(CScene* scene);
	void LoadScene(CScene* scene);

	void Save();
	inline CScene* GetScene() const { return scene; }

	inline bool IsActive() const { return bActive; }
	inline double CurTime() const { return time; }

	template<typename T>
	T* CreateEntity(const FString& name = FString()) { return (T*)CreateEntity(T::StaticClass(), name); }

	template<typename T>
	T* CreateEntity(const FString& name, const FVector& position, const FQuaternion& rot = FQuaternion(), const FVector& scale = FVector(1));

	CEntity* CreateEntity(FClass* classType, const FString& name);

	CEntity* GetEntity(const FString& name);
	CEntity* GetEntity(SizeType entityId);

	inline CGameMode* GetGameMode() const { return gamemode; }
	void SetGameMode(const TObjectPtr<CGameMode>& gm);

	inline bool IsInitialized() const { return bInitialized; }

	inline bool IsPrimary() const { return this == gWorld; }
	inline bool IsChildWorld() const { return parent != nullptr; }

	inline void SetRenderScene(CRenderScene* scene) { renderScene = scene; }
	inline CRenderScene* GetRenderScene() const { return renderScene; }

	inline IPhysicsWorld* GetPhysicsWorld() const { return physicsWorld; }

	inline void SetRenderWindow(IBaseWindow* wnd) { renderWindow = wnd; }
	inline IBaseWindow* GetRenderWindow() const { return renderWindow; }

	template<typename T>
	TArray<TObjectPtr<T>> FindEntitiesOfType();

	inline const TMap<SizeType, TObjectPtr<CEntity>>& GetEntities() const { return entities; }
	inline const TArray<TObjectPtr<CEntity>>& GetDynamicEntities() const { return dynamicEntities; }

	void Start();
	void Stop();
	void Update(double dt);

	void Render();

public:
	inline void RegisterPrimitive(CPrimitiveProxy* proxy) { primitives.Add(proxy); }
	inline void UnregisterPrimitive(CPrimitiveProxy* proxy) { if (auto it = primitives.Find(proxy); it != primitives.end()) primitives.Erase(it); }
	inline const TArray<CPrimitiveProxy*>& GetPrimitives() const { return primitives; }

	inline void RegisterLight(CLightProxy* proxy) { lights.Add(proxy); }
	inline void UnregisterLight(CLightProxy* proxy) { if (auto it = lights.Find(proxy); it != lights.end()) lights.Erase(it); }
	inline const TArray<CLightProxy*>& GetLights() const { return lights; }

	inline void RegisterCamera(CCameraProxy* proxy) { cameras.Add(proxy); }
	inline void UnregisterCamera(CCameraProxy* proxy) { if (auto it = cameras.Find(proxy); it != cameras.end()) cameras.Erase(it); }
	inline const TArray<CCameraProxy*>& GetCameras() const { return cameras; }
	
	inline void RegisterPPVolume(CPostProcessVolumeProxy* proxy) { ppVolumes.Add(proxy); }
	inline void UnregisterPPVolume(CPostProcessVolumeProxy* proxy) { if (auto it = ppVolumes.Find(proxy); it != ppVolumes.end()) ppVolumes.Erase(it); }

	inline CCameraProxy* GetPrimaryCamera() const { return primaryCamera; }
	inline void SetPrimaryCamera(CCameraProxy* cam) { primaryCamera = cam; }

protected:
	void OnDelete() override;

	void RemoveEntity(CEntity* ent);

	CEntityIOManager* GetEntityIOManager() const;

	void RegisterDynamicEntity(CEntity* ent);
	void RemoveDynamicEntity(CEntity* ent);

public: // Events
	TDelegate<CEntity*> OnEntityCreated;
	TDelegate<CEntity*> OnEntityDeleted;

protected:
	InitializeInfo initInfo;

	CWorld* parent = nullptr;
	TArray<CWorld*> subWorlds;

	//   EntityId, EntityPtr
	TMap<SizeType, TObjectPtr<CEntity>> entities;

	// The window that this scene gets rendered to.
	IBaseWindow* renderWindow = nullptr;

	TArray<TObjectPtr<CEntity>> dynamicEntities;

	TObjectPtr<CGameMode> gamemode;

	TArray<CLightProxy*> lights;
	TArray<CPrimitiveProxy*> primitives;
	TArray<CCameraProxy*> cameras;
	TArray<CPostProcessVolumeProxy*> ppVolumes;

	CCameraProxy* primaryCamera = nullptr;

	TObjectPtr<CScene> scene;
	CRenderScene* renderScene = nullptr;

	TObjectPtr<IPhysicsWorld> physicsWorld;

	// Entity IO Data
	CEntityIOManager* entityIOManager = nullptr;

	ENetMode netMode;

	double time;
	bool bStreaming : 1; // Wether we're currently streaming in a scene.
	bool bLoaded : 1; // Wether all entities have been loaded.
	bool bLoading : 1;
	bool bInitialized : 1;
	bool bActive : 1; // Is the world currently active and running.

};

template<typename T>
T* CWorld::CreateEntity(const FString& name, const FVector& position, const FQuaternion& rot /*= FQuaternion()*/, const FVector& scale /*= FVector()*/)
{
	T* ent = CreateEntity<T>(name);

	ent->SetWorldPosition(position);
	ent->SetWorldRotation(rot);
	ent->SetWorldScale(scale);
	return ent;
}

template<typename T>
TArray<TObjectPtr<T>> CWorld::FindEntitiesOfType()
{
	TArray<TObjectPtr<T>> r;
	for (auto& ent : entities)
	{
		if (auto c = Cast<T>(ent.second); c.IsValid())
			r.Add(c);
	}

	return r;
}

/*
 *	Helper class for registering objects to a world.
*/
class ENGINE_API FWorldRegisterer
{
public:
	static void UnregisterEntity(CWorld* world, CEntity* ent);
};
