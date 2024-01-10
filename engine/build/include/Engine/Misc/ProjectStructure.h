#pragma once

#include "EngineCore.h"
#include "Object/Class.h"

class CGameInstance;
class FLibrary;
struct FMod;

struct ENGINE_API FDependency
{
	enum EType
	{
		INVALID,
		LIBRARY,
		ADDON,
		GAME_ADDON,
	};

	EType type;
	FString name;
	void* instance = nullptr; // currently only used for type ::LIBRARY
};

struct ENGINE_API FAddon
{
	enum EType
	{
		CORE_ADDON,
		GAME_ADDON,
		INVALID_ADDON
	};

	FString identity;
	FString name;
	FString path;

	FString description;
	FString category;
	FString author;

	EType type;

	bool bHasCode;
	bool bHasContent;
	bool bShipSource;
	bool bInProjectFolder;

	CModule* module = nullptr;
	FMod* mod;
	TArray<FDependency> dependencies;
};

struct ENGINE_API FGame
{
	FString title;
	FString name;
	FString version;

	FMod* mod;

	FString startupScene;
	TClassPtr<CGameInstance> gameInstanceClass;
	TClassPtr<CInputManager> inputManagerClass;
};

struct ENGINE_API FProject
{
public:
	FString name;
	FString displayName;
	FString dir;
	FString author;
	FString game;

	TArray<FString> addons;	/* A list of Addons used in this project. */
	TArray<FAddon> projectAddons;

public:
	bool bIncludesSdk;		// does this project include an sdk.
	bool bHasEngineContent; // wether the engine content is shipped with this project.
	
};
