#pragma once

#include "EngineCore.h"
#include "Misc/ProjectStructure.h"
#include "Rendering/Renderer.h"
#include "Engine.generated.h"

class CEngine;
class CWindow;
class CRenderScene;
class CGameInstance;
class CInputManager;

extern ENGINE_API CEngine* gEngine;

extern ENGINE_API bool gIsClient;
extern ENGINE_API bool gIsServer;
extern ENGINE_API bool gIsRunning;
extern ENGINE_API bool gIsEditor;
extern ENGINE_API bool gIsMainGaurded;

struct FUserConfig
{
	int windowPosX, windowPosY;
	int windowWidth = 1920, windowHeight = 1080;
	uint8 windowMode;
	bool bVSync = true;
};

CLASS(Abstract, Hidden)
class ENGINE_API CEngine : public CObject
{
	GENERATED_BODY()
	
public:
	CEngine() = default;
	CEngine(const CEngine&) = delete;
	
	/**
	 * Starts the engine and Initializes mandetory services.
	 */
	void InitMinimal();

	/**
	 * Starts the engine and Intializes services (Renderer, Window, etc).
	 */
	virtual void Init();

	/**
	 * Loads the specified game.
	 *
	 * @param bFirst - used to tell if this is the first call during recursion. must always be true when called manually.
	 */
	void LoadGame();

	/**
	* Create world from the specified scene.
	* 
	* @param scene - the target scene file name, if the string is equal to "empty" (the default value), it will create an empty world instead.
	*/
	void LoadWorld(const WString& scene = L"empty", bool bImmediate = false);

	/**
	* Stream in a world from the specified scene.
	* 
	* 
	*/
	//void* StreamWorld(const FString& scene);

	/**
	 * Runs the Game loop.
	 */
	virtual int Run();

	virtual bool LoadProject(const WString& path = L".");

	void LoadAddon(const FAddon& addon);

	void Exit();

	void SaveUserConfig();
	void SaveConsoleLog();

#ifdef IS_DEV
	void HotReloadModule(const FString& module);
#endif

	static WString OSGetEnginePath(const FString& version);

public:
	inline CWindow* GetGameWindow() const { return gameWindow; }

	inline TEvent<>& OnHotReloadEvent() { return eventOnHotReload; }

	inline const FProject& GetProjectConfig() const { return projectConfig; }
	inline const FGame& ActiveGame() const { return activeGame; }

	inline CGameInstance* GameInstance() const { return gameInstance; }

	inline CInputManager* InputManager() const { return inputManager; }

	template<class T>
	inline T* SetGameInstance() { return (T*)SetGameInstance(T::StaticClass()); }
	CGameInstance* SetGameInstance(FClass* type);

	inline double GetUpdateTime() const { return updateTime; }
	inline double GetRenderTime() const { return renderTime; }

	inline double DeltaTime() const { return deltaTime; }

	inline WString GetGameConfigPath() const { return ToWString(activeGame.name) + L"\\config"; }
	inline WString EngineContentPath() const { return engineMod->Path(); }

protected:
	virtual void OnExit();

	void DoLoadWorld();

	bool LoadProjectConfig(const WString& path);
	bool LoadUserConfig();

	// Finds and loads core addons.
	void LoadCoreAddons();

	void RegisterAddon(const WString& path);

public:
	FUserConfig userConfig;
	
protected:
	TEvent<> eventOnHotReload;

	WString nextSceneName;

	bool bWantsToExit = false;
	bool bInitialized = false;

	FMod* engineMod = nullptr;

	double _time;
	double _prevTime;
	double deltaTime;

	double updateTime;
	double renderTime;

	TObjectPtr<CGameInstance> gameInstance;
	TObjectPtr<CInputManager> inputManager;

	FProject projectConfig;
	FGame activeGame;

	TArray<FAddon> coreAddons;

	//CRenderScene* worldRenderScene;

	CWindow* gameWindow;
};