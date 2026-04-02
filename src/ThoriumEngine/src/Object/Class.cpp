
#include "Class.h"
#include "Object.h"
#include "PropertyHandler.h"
#include "PropertyTypes.h"

const char* VariableTypeToString(uint8 type)
{
	const char* txt[] = {
		"EVT_NULL",
		"EVT_VOID",
		"EVT_STRUCT",
		"EVT_CLASS",
		"EVT_STRING",
		"EVT_ENUM",
		"EVT_ARRAY",
		"EVT_OBJECT_PTR",

		"EVT_CLASS_PTR",
		"EVT_STRUCT_PTR",
		"EVT_ENUM_PTR",

		"EVT_FLOAT",
		"EVT_DOUBLE",

		"EVT_INT",
		"EVT_INT8",
		"EVT_INT16",
		"EVT_INT64",

		"EVT_UINT",
		"EVT_UINT8",
		"EVT_UINT16",
		"EVT_UINT64",

		"EVT_BOOL",
	};
	return txt[type];
}

int64_t FEnum::GetValueByName(const FString& name)
{
	for (auto& v : values)
	{
		if (v.Key == name)
			return v.Value;
	}
	return -1;
}

FString FEnum::GetNameByValue(int64 val)
{
	for (auto& v : values)
	{
		if (v.Value == val)
			return v.Key;
	}

	return FString();
}

const FProperty* FStruct::GetProperty(const FString& name) const
{
	for (const FProperty* prop = PropertyList; prop != nullptr; prop = prop->next)
		if (prop->cppName == name)
			return prop;

	return nullptr;
}

void* FStruct::GetDefaultObject()
{
	if (!defaultObject)
	{
		THORIUM_ASSERT((flags & CTAG_CLASS) == 0, "FStruct::GetDefaultValue cannot return object of type CObject!");

		defaultObject = malloc(size);
		if (constructor)
			constructor((CObject*)defaultObject);
		else
			memset(defaultObject, 0, size);
	}

	return defaultObject;
}

CObject* FClass::GetDefaultObject()
{
	if (!defaultObject)
	{
		THORIUM_ASSERT((flags & CTAG_CLASS) != 0, "FClass::GetDefaultValue cannot return object of type struct!");

		CObject* obj = CreateObject(this, "default_" + name);
		if (obj)
			obj->MakeIndestructible();
		defaultObject = obj;
	}
	return (CObject*)defaultObject;
}

const FProperty* FClass::GetProperty(const FString& name) const
{
	for (const FProperty* prop = PropertyList; prop != nullptr; prop = prop->next)
		if (prop->cppName == name)
			return prop;

	return baseType ? baseType->GetProperty(name) : nullptr;
}

const FFunction* FClass::GetFunction(const FString& name) const
{
	for (const FFunction* func = FunctionList; func != nullptr; func = func->next)
		if (func->cppName == name)
			return func;

	return nullptr;
}

const FFunction* FClass::GetFunction(SizeType id)
{
	for (const FFunction* func = FunctionList; func != nullptr; func = func->next)
	{
		if (func->id == id)
			return func;
	}
	return nullptr;
}

bool FClass::HasTag(const FString& tag)
{
	for (uint32 i = 0; i < numTags; i++)
		if (tags[i].Key == tag)
			return true;

	return false;
}

FString FClass::TagValue(const FString& key)
{
	for (uint32 i = 0; i < numTags; i++)
		if (tags[i].Key == key)
			return tags[i].Value;

	return FString();
}

bool FClass::CanCast(FClass* castTo)
{
	if (castTo == this)
		return true;

	if (baseType)
		return ((FClass*)baseType)->CanCast(castTo);

	return false;
}

FString FPropertyMeta::FlagValue(const FString& key)
{
	for (uint32 i = 0; i < numGenericFlags; i++)
		if (genericFlags[i].Key == key)
			return genericFlags[i].Value;

	return FString();
}

bool FPropertyMeta::HasFlag(const FString& key)
{
	for (uint32 i = 0; i < numGenericFlags; i++)
		if (genericFlags[i].Key == key)
			return true;

	return false;
}

IPropertyHandler* FProperty::GetHandler(void* obj) const
{
	switch (type)
	{
	case EVT_OBJECT_PTR:
	case EVT_CLASS:
		return new FObjectPtrPropertyHandler(obj, this);
	case EVT_CLASS_PTR:
		return new FClassPtrPropertyHandler(obj, this);
	case EVT_STRING:
		return new FStringPropertyHandler(obj, this);
	case EVT_ARRAY:
		return new FArrayPropertyHandler(obj, this);
	case EVT_STRUCT:
		return new FStructPropertyHandler(obj, this);
	}

	return new FDefaultPropertyHandler(obj, this);
}

IPropertyHandler* FProperty::GetHandler(CObject* obj) const
{
	if (type == EVT_CLASS || type == EVT_OBJECT_PTR)
		return new FObjectPtrPropertyHandler(obj, this);

	return GetHandler((void*)obj);
}

bool operator==(const FArgType& a, const FArgType& b)
{
	if (a.bConst == b.bConst && a.bPointer == b.bPointer && a.bRef == b.bRef && a.numTemplates == b.numTemplates && a.size == b.size && a.type == b.type)
	{
		if (a.typeName != b.typeName)
			return false;
		for (uint8 i = 0; i < a.numTemplates; i++)
			if (!(a.templateType[i] == b.templateType[i]))
				return false;
		return true;
	}

	return false;
}

bool operator==(const FProperty& a, const FArgType& b)
{
	FArgType aType = { a.type, a.typeName, a.size, a.numTemplates, a.templateType, false, false, a.flags & VTAG_TYPE_POINTER };
	return aType == b;
}
