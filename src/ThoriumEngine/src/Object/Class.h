#pragma once

class CObject;

#include <Util/Core.h>
#include "Misc/Script.h"
#include "ObjectTypes.h"
#include "PropertyTypes.h"
#include "Module.h"

#include "EngineCore.h"

class FClass;

typedef void(*FFunctionExecPtr)(CObject* target, FStack& stack);

struct FBaseField
{
	FString name;
	FString cppName;
#if defined(INCLUDE_EDITOR_DATA)
	FString description;
#endif
};

struct ENGINE_API FPropertyMeta
{
public:
	bool HasFlag(const FString& flag);
	FString FlagValue(const FString& key);

public:
	FString uiMin;
	FString uiMax;
	FString category;
	FString defaultValue;

	//FFunctionExecPtr onEditFunc;

	SizeType numGenericFlags;
	TPair<FString, FString>* genericFlags;
};

struct FProperty : public FBaseField
{
	FString typeName;

	uint type;
	uint flags;
	SizeType offset;
	SizeType size;

	FPropertyMeta* meta;
	void* typeHelper;
	FProperty* next;
};

struct FFuncArg
{
	FString name;
	uint type;
	uint8 flags;
	void* classPtr;
};

struct FFunction : public FBaseField
{
	enum EType
	{
		GENERAL,
		INPUT,
		COMMAND,
		SERVER_RPC,
		CLIENT_RPC,
		MULTICAST_RPC
	};

	FFunctionExecPtr execFunc;
	EType type;

	TArray<FFuncArg> Arguments;

	bool bStatic : 1;
	FFunction* next;
};

enum EEnumFlags : uint8
{
	EnumFlag_NONE,
	EnumFlag_IS_FLAG = 1 << 0,
};

enum EClassType : uint8
{
	ClassType_Enum,
	ClassType_Struct,
	ClassType_Class
};

template<class T>
class TClassPtr
{
public:
	TClassPtr() = default;
	TClassPtr(const TClassPtr<T>& other);
	TClassPtr(const FString& type);
	TClassPtr(FClass* c);

	inline FClass* Get() const { return ptr; }

private:
	FClass* ptr = nullptr;
};

class ENGINE_API FEnum : public FBaseField
{
public:
	inline const FString& GetName() const { return name; }
	inline const FString& GetInternalName() const { return cppName; }
	inline const FString& GetDescription() const { return description; }

	// The size of the enum in bytes.
	inline SizeType Size() const { return size; }
	
	// The amount of values in the enum
	inline SizeType ValueCount() const { return values.Size(); }
	
	inline EEnumFlags Flags() const { return flags; }

	inline const TArray<TPair<FString, int64_t>>& GetValues() const { return values; }
	inline const FString& GetNameByIndex(SizeType index) { return values[index].Key; }
	inline int64_t GetValueByIndex(SizeType index) { return values[index].Value; }
	int64_t GetValueByName(const FString& name);
	
protected:
	TArray<TPair<FString, int64_t>> values;

	SizeType size;

	EEnumFlags flags;

};

class ENGINE_API FStruct : public FBaseField
{
public:
	inline const FString& GetName() const { return name; }
	inline const FString& GetInternalName() const { return cppName; }
	inline const FString& GetDescription() const { return description; }
	inline SizeType Size() const { return size; }
	inline uint32 NumProperties() const { return numProperties; }
	inline const FProperty* GetPropertyList() const { return PropertyList; }
	inline bool IsClass() const { return bIsClass; }

protected:
	SizeType size;

	bool bIsClass;

	uint32 numProperties;
	const FProperty* PropertyList;
};

/**
 * Class meta data object.
 */
class ENGINE_API FClass : public FStruct
{
public:
	virtual CObject* Instantiate() = 0;

public:
	inline uint32 NumFunctions() const { return numFunctions; }
	inline const FFunction* GetFunctionList() const { return FunctionList; }

	const FFunction* GetFunction(const FString& name);

	inline uint Flags() const { return flags; }
	inline bool HasFlag(uint f) const { return (flags & f); }

	inline FClass* GetBaseClass() const { return BaseClass; }

	bool HasTag(const FString& tag);
	FString TagValue(const FString& key);

	bool CanCast(FClass* castTo)
	{
		if (castTo == this)
			return true;

		if (BaseClass)
			return BaseClass->CanCast(castTo);

		return false;
	}

protected:
	FClass* BaseClass = nullptr;

	uint32 numFunctions;
	const FFunction* FunctionList;

	uint flags;

	uint32 numTags;
	TPair<FString, FString>* tags;
};

class ENGINE_API FAssetClass : public FClass
{
public:
	inline const FString& GetExtension() const { return extension; }
	inline uint AssetFlags() const { return assetFlags; }

	inline const FString& ImportableAs() const { return importableAs; }

protected:
	FString extension;
	uint assetFlags;

	// List of file types that this asset can be convert from. example: ".fbx;.png;.obj;..."
	FString importableAs; 

};

template<class T>
TClassPtr<T>::TClassPtr(FClass* c)
{
	if (c->CanCast(T::StaticClass()))
		ptr = c;
}

template<class T>
TClassPtr<T>::TClassPtr(const TClassPtr<T>& other) : ptr(other.ptr)
{
}

template<class T>
TClassPtr<T>::TClassPtr(const FString& type)
{
	FClass* c = CModuleManager::FindClass(type);
	if (c && c->CanCast(T::StaticClass()))
		ptr = c;
}
