#pragma once

#include "EngineCore.h"
#include "Object/Class.h"

class CGameInstance;
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
};

struct ENGINE_API FAddon
{
	enum EType
	{
		CORE_ADDON,
		GAME_ADDON
	};

	FString identity;
	FString name;
	WString path;

	FString description;
	FString category;
	FString author;

	EType type;

	bool bHasContent;
	bool bShipSource;
	bool bInProjectFolder;

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

	TArray<FAddon> addons;
};

struct ENGINE_API FProject
{
public:
	FString name;
	FString dispalyName;
	WString dir;
	FString author;
	FString game;

	TArray<FString> addons;	/* A list of Addons used in this project. */

public:
	bool bIncludesSdk;		// does this project include an sdk.
	bool bHasEngineContent; // wether the engine content is shipped with this project.
	
};
