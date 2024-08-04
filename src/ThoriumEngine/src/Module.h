#pragma once

#include "EngineCore.h"
#include <Util/Map.h>

class CModuleManager;
class FClass;
class FStruct;
class FEnum;
class FAssetClass;

// Generic OS library (e.g. DLL)
class ENGINE_API FLibrary
{
	friend class CEngine;
	friend class CModuleManager;

	typedef void* FuncAdress;

public:
	inline const FString& Name() const { return name; }
	inline const FString& Path() const { return path; }

	inline void* GetHandle() const { return handle; }

	FuncAdress GetFunctionPtr(const FString& fun);

protected:
	FString name;
	FString path;

	void* handle;
};

class ENGINE_API CModule : public FLibrary
{
	friend class CEngine;
	friend class CModuleManager;

public:
	CModule(const char* _name);

	void RegisterFClass(FClass* c);
	void RegisterFStruct(FStruct* c);
	void RegisterFEnum(FEnum* c);
	void RegisterFAsset(FAssetClass* c);

protected:
	//FStruct* StructList;
	//FClass* ClassList;
	//FEnum* EnumList;

public:
	TMap<SizeType, FStruct*> Structures;
	TMap<SizeType, FClass*> Classes;
	TMap<SizeType, FEnum*> Enums;
	TArray<FAssetClass*> Assets;

};

class ENGINE_API CModuleManager
{
	friend class CEngine;
	friend class CEditorEngine;

public:
	static FClass* GetClass(const FString& name);
	static FStruct* GetStruct(const FString& name);
	static FEnum* GetEnum(const FString& name);

	static FClass* GetClass(SizeType id);
	static FStruct* GetStruct(SizeType id);
	static FEnum* GetEnum(SizeType id);

	static void GetAssetTypes(TArray<FAssetClass*>& out);

	static void FindChildClasses(FClass* target, TArray<FClass*>& out);
	static void GetClassesOfType(FClass* type, TArray<FClass*>& out);

	static int LoadModule(const FString& path, CModule** outPtr = nullptr);
	static inline bool IsModuleLoaded(const FString& name) { return FindModule(name) != nullptr; }
	static bool UnloadModule(const FString& name);
	static bool UnloadModule(CModule* module);

	// Thanks to the lovely windows API, I can't name this LoadLibrary.
	static FLibrary* LoadFLibrary(const FString& name, const FString& path);
	static bool UnloadLibrary(FLibrary* lib);
	static bool UnloadLibrary(const FString& name);

	static const TArray<CModule*>& GetModules() { return modules; }

private:
	static void Cleanup();

	static CModule* FindModule(const FString& name);
	static inline void RegisterModule(CModule* m) { modules.Add(m); }

private:
	static TArray<CModule*> modules;
	static TArray<FLibrary*> libraries;

};
