#pragma once

#include <Util/Core.h>
#include <Util/Event.h>
#include "Game/Entity.h"
#include "EngineCore.h"
#include "Object/Delegate.h"
#include <mutex>

#include "World.generated.h"

class CWorld;
class CScene;
class IBaseWindow;
class CPrimitiveProxy;
class CLightProxy;
class CCameraProxy;
class CRenderScene;
class CGameMode;

extern ENGINE_API CWorld* gWorld;

enum ENetMode
{
	NetMode_Singeplayer, // A Standalone game with no networking.
	NetMode_Dedicated, // A Dedicated server which clients can join.
	NetMode_Host, // A Client running a server which other clients can join.
	NetMode_Client // A Client connected to a server.
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
	inline CWorld() : bInitialized(0), bActive(0), time(0.f) { bIndestructible = true; }
	
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
	T* CreateEntity(const FString& name = "") { return (T*)CreateEntity(T::StaticClass(), name); }

	CEntity* CreateEntity(FClass* classType, const FString& name);

	inline CGameMode* GetGameMode() const { return gamemode; }
	void SetGameMode(const TObjectPtr<CGameMode>& gm);

	inline bool IsInitialized() const { return bInitialized; }

	inline bool IsPrimary() const { return this == gWorld; }
	inline bool IsChildWorld() const { return parent != nullptr; }

	inline void SetRenderScene(CRenderScene* scene) { renderScene = scene; }
	inline CRenderScene* GetRenderScene() const { return renderScene; }

	inline void SetRenderWindow(IBaseWindow* wnd) { renderWindow = wnd; }
	inline IBaseWindow* GetRenderWindow() const { return renderWindow; }

	template<typename T>
	TArray<TObjectPtr<T>> FindEntitiesOfType();

	inline const TArray<TObjectPtr<CEntity>>& GetEntities() const { return entities; }

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
	
	inline CCameraProxy* GetPrimaryCamera() const { return primaryCamera; }
	inline void SetPrimaryCamera(CCameraProxy* cam) { primaryCamera = cam; }

protected:
	void OnDelete() override;

	void RemoveEntity(CEntity* ent);

public: // Events
	TDelegate<CEntity*> OnEntityCreated;
	TDelegate<CEntity*> OnEntityDeleted;

protected:
	InitializeInfo initInfo;

	CWorld* parent;
	TArray<CWorld*> subWorlds;
	TArray<TObjectPtr<CEntity>> entities;

	// The window that this scene gets rendered to.
	IBaseWindow* renderWindow;

	TObjectPtr<CGameMode> gamemode;

	TArray<CLightProxy*> lights;
	TArray<CPrimitiveProxy*> primitives;
	TArray<CCameraProxy*> cameras;

	CCameraProxy* primaryCamera;

	TObjectPtr<CScene> scene;
	CRenderScene* renderScene;

	ENetMode netMode;

	double time;
	bool bStreaming : 1; // Wether we're currently streaming in a scene.
	bool bLoaded : 1; // Wether all entities have been loaded.
	bool bInitialized : 1;
	bool bActive : 1; // Is the world currently active and running.

};

template<typename T>
TArray<TObjectPtr<T>> CWorld::FindEntitiesOfType()
{
	TArray<TObjectPtr<T>> r;
	for (auto& ent : entities)
	{
		if (auto c = Cast<T>(ent); c.IsValid())
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
