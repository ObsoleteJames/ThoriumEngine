#pragma once

#include "EngineCore.h"

class CModuleManager;
class FClass;
class FStruct;
class FEnum;
class FAssetClass;

class ENGINE_API CModule
{
	friend class CEngine;
	friend class CModuleManager;

public:
	CModule(const char* _name) { name = _name; }

	inline const FString& Name() const { return name; }
	inline const WString& Path() const { return path; }

	inline void RegisterFClass(FClass* c) { Classes.Add(c); }
	inline void RegisterFStruct(FStruct* c) { Structures.Add(c); }
	inline void RegisterFEnum(FEnum* c) { Enums.Add(c); }
	inline void RegisterFAsset(FAssetClass* c) { Assets.Add(c); }

protected:
	FString name;
	WString path;

	//FStruct* StructList;
	//FClass* ClassList;
	//FEnum* EnumList;

public:
	TArray<FStruct*> Structures;
	TArray<FClass*> Classes;
	TArray<FEnum*> Enums;
	TArray<FAssetClass*> Assets;

private:
	void* moduleHandle;

};

class ENGINE_API CModuleManager
{
	friend class CEngine;
	friend class CEditorEngine;

public:
	static FClass* FindClass(const FString& name);
	static FStruct* FindStruct(const FString& name);
	static FEnum* FindEnum(const FString& name);

	static void GetAssetTypes(TArray<FAssetClass*>& out);

	static void FindChildClasses(FClass* target, TArray<FClass*>& out);

	static int LoadModule(const WString& path);
	static inline bool IsModuleLoaded(const FString& name) { return FindModule(name) != nullptr; }

	static bool UnloadModule(const FString& name);

	static const TArray<CModule*>& GetModules() { return modules; }

private:
	static void Cleanup();

	static CModule* FindModule(const FString& name);
	static inline void RegisterModule(CModule* m) { modules.Add(m); }

private:
	static TArray<CModule*> modules;

};
