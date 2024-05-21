#pragma once

#include "EngineCore.h"
#include "Misc/ProjectStructure.h"
#include "Rendering/Renderer.h"
#include "Physics/PhysicsSettings.h"
#include "Engine.generated.h"

class CEngine;
class CWindow;
class CRenderScene;
class CGameInstance;
class CInputManager;

extern ENGINE_API CEngine* gEngine;

extern ENGINE_API bool bIsTerminal;
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
	 * starts the engine as a terminal application
	 */
	virtual void InitTerminal();

	/**
	 * Loads the specified game.
	 */
	void LoadGame();

	/**
	* Create world from the specified scene.
	* 
	* @param scene - the target scene file name, if the string is equal to "empty" (the default value), it will create an empty world instead.
	*/
	void LoadWorld(const FString& scene = "empty", bool bImmediate = false);

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

	virtual bool LoadProject(const FString& path = ".");

	/**
	 * Unloads the current project.
	 */
	virtual bool UnloadProject();

	// Load the addon's content and or module.
	void LoadAddon(FAddon& addon);

	void UnloadAddon(FAddon& addon);

	void LoadCoreAddon(const FString& id);

	void Exit();

	void SaveUserConfig();
	void SaveConsoleLog();

#ifdef IS_DEV
	void HotReloadModule(const FString& module);
#endif

	static FString OSGetEnginePath(const FString& version);
	static FString OSGetDataPath();
	static FString OSGetDocumentsPath();

	static FString OpenFileDialog(const FString& filter = FString());
	static FString SaveFileDialog(const FString& filter = FString());
	static FString OpenFolderDialog();

	static int ExecuteProgram(const FString& cmd, bool bWaitForExit = true);

public:
	inline CWindow* GetGameWindow() const { return gameWindow; }

	inline TEvent<>& OnHotReloadEvent() { return eventOnHotReload; }

	inline FProject& GetProjectConfig() { return projectConfig; }
	inline FGame& ActiveGame() { return activeGame; }
	inline bool IsProjectLoaded() const { return bProjectLoaded; }

	inline CGameInstance* GameInstance() const { return gameInstance; }

	inline CInputManager* InputManager() const { return inputManager; }

	template<class T>
	inline T* SetGameInstance() { return (T*)SetGameInstance(T::StaticClass()); }
	CGameInstance* SetGameInstance(FClass* type);

	void CreatePhysicsApi(FClass* type);

	inline double GetUpdateTime() const { return updateTime; }
	inline double GetRenderTime() const { return renderTime; }

	inline double DeltaTime() const { return deltaTime; }

	inline FString GetGameConfigPath() const { return activeGame.name + "/config"; }
	inline FString EngineContentPath() const { return engineMod->Path(); }

	inline const TArray<FAddon>& GetAddons() const { return coreAddons; }

protected:
	virtual void OnExit();

	void InitImGui();

	void DoLoadWorld();

	bool LoadProjectConfig(const FString& path, FProject& outProject);
	bool LoadUserConfig();

	void FetchAddons(const FString& addonFolder, TArray<FAddon>& out);

	bool LoadAddonConfig(const FString& path, FAddon& out);

	void LoadMandatoryAddons();

	void UnloadWorld();

public:
	FUserConfig userConfig;
	
protected:
	TEvent<> eventOnHotReload;

	FString nextSceneName;

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

	bool bProjectLoaded = false;
	FProject projectConfig;
	FGame activeGame;
	FPhysicsSettings physicsSettings;

	TArray<FAddon> coreAddons;

	//CRenderScene* worldRenderScene;

	CWindow* gameWindow = nullptr;
};
