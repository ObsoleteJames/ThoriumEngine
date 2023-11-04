
#include "Class.h"

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
	for (const FFunction* func = FunctionList; func != nullptr; func = FunctionList->next)
		if (func->cppName == name)
			return func;

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
