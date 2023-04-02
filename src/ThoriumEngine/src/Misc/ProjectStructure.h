#pragma once

#include "EngineCore.h"
#include "Object/Class.h"

class CGameInstance;
struct FMod;

struct ENGINE_API FGame
{
	FString title;
	FString name;
	FString version;

	FMod* mod;

	FString startupScene;
	TClassPtr<CGameInstance> gameInstanceClass;

	TArray<FString> dependencies;
	TArray<FString> addons;
};

struct ENGINE_API FProject
{
public:
	FString name;
	FString dispalyName;
	WString dir;
	FString author;
	FString defaultGame;

	TArray<FString> games;	/* The Games available in this project. */
	//TArray<FString> addons;	/* A list of all Addons available in this project. */

public:
	bool bIncludesSdk;		// does this project include an sdk.
	bool bHasEngineContent; // wether the engine content is shipped with this project.
	
};
