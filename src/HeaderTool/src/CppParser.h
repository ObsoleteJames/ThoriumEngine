#pragma once

#include "CppTypes.h"

struct FHeaderData
{
	FString FilePath;
	FString FileName;
	bool bEmpty = false;

	TArray<CppClass> classes;
	TArray<CppEnum> enums;
};

struct FTypeDefinition
{
	FString name;
	FString moduleName;
	uint8 type; // 0 = struct, 1 = class, 2 = enum.
};

enum EProjectType
{
	ENGINE_DLL,
	GAME_PROJECT,
	DLC_PROJECT,
	LIBRARY_PROJECT
};

extern EProjectType ProjectType;

extern FString GeneratedOutput;

//extern FString config;
//extern FString platform;
extern FString targetPath;
extern FString projectName;

extern TArray<FTypeDefinition> PreRegisteredClasses;
extern TArray<FHeaderData> Headers;

class CParser
{
public:
	static int ParseHeader(FHeaderData& data);

	static void WriteModuleCpp();
	static void WriteGeneratedHeader(const FHeaderData& data);
	static void WriteGeneratedCpp(const FHeaderData& data);

	static void LoadModuleData(const FString& projectPath);
	static bool WriteModuleData();
	static bool HeaderUpToDate(FHeaderData& header);
	static void WriteTimestamp();

};
