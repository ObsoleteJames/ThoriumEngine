#pragma once

class CObject;

#include <Util/Core.h>
#include "Misc/Script.h"
#include "ObjectTypes.h"
#include "Module.h"

#include "EngineCore.h"

class FClass;
class IPropertyHandler;

typedef void(*FFunctionExecPtr)(CObject* target, FStack& stack);

struct ENGINE_API FBaseField
{
	FString name;
	FString cppName;
	FString description;

	SizeType id;
};

struct ENGINE_API FArgType
{
	EVariableType type;
	FString typeName;
	SizeType size; // size of the type.

	// number of template types.
	uint8 numTemplates;
	FArgType* templateType;

	bool bConst;
	bool bRef;
	bool bPointer;
};

bool operator==(const FArgType& a, const FArgType& b);
inline bool operator!=(const FArgType& a, const FArgType& b) { return !(a == b); }

struct ENGINE_API FPropertyMeta
{
public:
	bool HasFlag(const FString& flag);
	FString FlagValue(const FString& key);

public:
	FString category;
	
	SizeType numGenericFlags;
	TPair<FString, FString>* genericFlags;
};

struct ENGINE_API FProperty : public FBaseField
{
public:
	IPropertyHandler* GetHandler(void* obj) const;
	IPropertyHandler* GetHandler(CObject* obj) const;

	template<class T>
	T* GetHandler(void* obj) const { return (T*)GetHandler(obj); }

public:
	uint8 protectionLvl;

	FString typeName;

	EVariableType type;

	// number of template types.
	uint8 numTemplates; 
	FArgType* templateType;

	EVariableFlags flags;
	SizeType offset;
	SizeType size;

	FPropertyMeta* meta;
	void* typeHandler;
	FProperty* next;
};

bool operator==(const FProperty& a, const FArgType& b);
inline bool operator!=(const FProperty& a, const FArgType& b) { return !(a == b); }

struct ENGINE_API FFuncArg
{
	FString name;
	FArgType type;
};

struct ENGINE_API FFunction : public FBaseField
{
	enum EType
	{
		GENERAL,
		OUTPUT,
		COMMAND,
		SERVER_RPC,
		CLIENT_RPC,
		MULTICAST_RPC,
		OPERATOR
	};

	uint8 protectionLvl;

	std::function<void(CObject*, FStack&)> execFunc;
	EType type;

	FArgType returnType;

	SizeType numArguments;
	FFuncArg* Arguments;

	EFunctionFlags flags;
	FPropertyMeta* meta;
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
	inline bool IsClass() const { return flags & CTAG_CLASS; }

	inline FStruct* GetBaseStruct() const { return baseType; }

	inline uint Flags() const { return flags; }
	inline bool HasFlag(uint f) const { return (flags & f); }

	const FProperty* GetProperty(const FString& name) const;

	// returns a pointer to the default object of this struct. 
	// For classes, this is the CDO.
	void* GetDefaultObject();

public:
	std::function<void(void*)> constructor;

protected:
	FStruct* baseType;
	void* defaultObject;

	SizeType size;

	uint flags;

	uint32 numProperties;
	const FProperty* PropertyList;
};

/**
 * Class meta data object.
 */
class ENGINE_API FClass : public FStruct
{
public:
	// this is no longer used. and will be removed in the future.
	virtual CObject* Instantiate() { return nullptr; }

public:
	inline uint32 NumFunctions() const { return numFunctions; }
	inline const FFunction* GetFunctionList() const { return FunctionList; }

	const FProperty* GetProperty(const FString& name) const;
	const FFunction* GetFunction(const FString& name) const;

	inline FClass* GetBaseClass() const { return (FClass*)baseType; }

	CObject* GetDefaultObject();

	bool HasTag(const FString& tag);
	FString TagValue(const FString& key);

	bool CanCast(FClass* castTo);

protected:
	uint32 numFunctions;
	const FFunction* FunctionList;

	uint32 numTags;
	TPair<FString, FString>* tags;
};

class ENGINE_API FAssetClass : public FClass
{
public:
	inline uint AssetFlags() const { return assetFlags; }

protected:
	uint assetFlags;
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
	FClass* c = CModuleManager::FindClass(type);
	if (c && c->CanCast(T::StaticClass()))
		ptr = c;
}
