#pragma once

#define CREATE_OBJECT_REGISTRY(RegistryClass, RegistryName) \
class RegistryName \
{ \
public:\
	static TArray<##RegistryClass>& Get() \
	{ \
		static TArray<##RegistryClass> regist; \
		return regist; \
	} \
	static void AddToRegistry(##RegistryClass obj) \
	{ \
		Get().Add(obj); \
	} \
}

#define CREATE_OBJECT_REGISTRY_DLL(RegistryName, RegistryClass, __API) \
class __API RegistryName \
{ \
public:\
	static TArray<##RegistryClass>& Get() \
	{ \
		static TArray<##RegistryClass> regist; \
		return regist; \
	} \
	static void AddToRegistry(##RegistryClass obj) \
	{ \
		Get().Add(obj); \
	} \
	static void Remove(##RegistryClass obj) \
	{ \
		Get().Erase(Get().Find(obj)); \
	} \
}
