#pragma once

#include "EngineCore.h"

#include <Util/Guid.h>
#include <Util/Map.h>

class CObject;
class FClass;
class FStruct;
class CModule;

class ENGINE_API CObjectManager
{
	friend class CObject;
	friend class CEngine;
	friend class CEditorEngine;

public:
	static inline const TMap<FGuid, CObject*>& GetAllObjects() { return Objects; }
	static CObject* FindObject(const FString& name);
	static CObject* FindObject(SizeType id);

	static CObject* FindObject(FClass* type);
	static TArray<CObject*> FindObjects(FClass* type);

	template<typename T>
	static inline T* FindObject() { return (T*)FindObject(T::StaticClass()); }

	template<typename T>
	static inline TArray<T*> FindObjects() { return (TArray<T*>)FindObjects(T::StaticClass()); }

protected:
	static void IdChanged(CObject* obj, SizeType oldId);
	static bool DeleteObject(CObject* obj, bool bNoErase = false);
	static void RegisterObject(CObject* obj);
	static void Update();
	static void Shutdown();

	// this could cause a crash :)
	static void DeleteObjectsFromModule(CModule* module);

#if OBJECT_KEEP_REFERENCES
	static bool ReplaceObject(CObject* target, CObject* replacement);
#endif

private:
	static TMap<FGuid, CObject*> Objects;
	static TArray<CObject*> ObjectsToDelete;

};
