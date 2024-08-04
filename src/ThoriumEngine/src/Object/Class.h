#pragma once

class CObject;

#include <Util/Core.h>
#include "ObjectTypes.h"
#include "Module.h"

#include "EngineCore.h"

class FClass;
class FArrayHelper;
struct FStack;

typedef uint8 EDataType;

typedef void(*FFunctionExecPtr)(CObject* target, FStack& stack);

struct FBaseField
{
	FString name;
	FString cppName;
#if defined(INCLUDE_EDITOR_DATA)
	FString description;
#endif

	size_t id;
};

struct FArgType
{
	uint type;
	size_t typeId;

	uint templateType;
	size_t templateTypeId;

	bool bConst : 1;
	bool bRef : 1;
	bool bPointer : 1;
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
	uint8 protectionLvl;

	FString typeName;

	EDataType type;
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
	FArgType type;
};

enum EFunctionFlags_
{
	FunctionFlags_NONE = 0,
	FunctionFlags_ALLOW_AS_INPUT = 1 << 0, // can this function be called from entity IO.
	FunctionFlags_STATIC = 1 << 1,
	FunctionFlags_SCRIPT_VIRTUAL = 1 << 2, // function that can be overriden in scripts.
	FunctionFlags_SCRIPT_CALLABLE = 1 << 3,
};

typedef uint EFunctionFlags;

struct FFunction : public FBaseField
{
	enum EType
	{
		GENERAL,
		OUTPUT,
		COMMAND,
		SERVER_RPC,
		CLIENT_RPC,
		MULTICAST_RPC,
		OPERATOR,
	};

	uint8 protectionLvl;

	//FFunctionExecPtr execFunc;
	std::function<bool(CObject* obj, FStack& stack)> execFunc;
	EType type;

	FArgType returnType;

	SizeType numArguments;
	FFuncArg* Arguments;

	EFunctionFlags flags;
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

	bool operator==(const TClassPtr<T>& r) { return r.ptr == ptr; }
	bool operator!=(const TClassPtr<T>& r) { return r.ptr != ptr; }

	bool operator==(FClass* r) { return r == ptr; }
	bool operator!=(FClass* r) { return r != ptr; }

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
	FString GetNameByValue(int64 v);
	
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

	const FProperty* GetProperty(SizeType id);

protected:
	SizeType size;

	bool bIsClass;
	bool bIsScriptType = 0; // Wether this type was compiled from a script.

	SizeType baseTypeId;

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
	const FFunction* GetFunction(SizeType id);

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
	inline uint AssetFlags() const { return assetFlags; }

	inline const FString& ImportableAs() const { return importableAs; }

	inline const FString& MetaExtension() const { return metaExt; }

protected:
	uint assetFlags;

	FString metaExt;

	// List of file types that this asset can be convert from. example: ".fbx;.png;.obj;..."
	FString importableAs;
};

template<class T>
TClassPtr<T>::TClassPtr(FClass* c)
{
	if (c && c->CanCast(T::StaticClass()))
		ptr = c;
}

template<class T>
TClassPtr<T>::TClassPtr(const TClassPtr<T>& other) : ptr(other.ptr)
{
}

template<class T>
TClassPtr<T>::TClassPtr(const FString& type)
{
	FClass* c = CModuleManager::GetClass(type);
	if (c && c->CanCast(T::StaticClass()))
		ptr = c;
}
