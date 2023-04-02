#pragma once

#include "EngineCore.h"
#include "Object.h"
#include "ObjectHandle.generated.h"

//template<typename T>
STRUCT()
struct ENGINE_API FObjectHandle
{
	GENERATED_BODY()

public:
	FObjectHandle() = default;
	FObjectHandle(CObject* ptr);
	FObjectHandle(const FObjectHandle& other);
	
	FObjectHandle& operator=(CObject* obj);
	
	template<typename T>
	inline T* Get() { return CastChecked<T>(CObjectManager::FindObject(objectId)); }

	inline CObject* Get() { return CObjectManager::FindObject(objectId); }

	bool operator==(const FObjectHandle& other) { return other.objectId == objectId; }
	bool operator==(CObject* obj) { return objectId == obj->Id(); }

public:
	PROPERTY()
	SizeType objectId = 0;
};
