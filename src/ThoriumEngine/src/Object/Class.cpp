
#include "Class.h"

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

const FFunction* FClass::GetFunction(const FString& name)
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

const FProperty* FStruct::GetProperty(SizeType id)
{
	for (const FProperty* prop = PropertyList; prop != nullptr; prop = prop->next)
	{
		if (prop->id == id)
			return prop;
	}
	return nullptr;
}
