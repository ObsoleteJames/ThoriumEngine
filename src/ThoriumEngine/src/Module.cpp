
#include "Module.h"
#include "Object/Class.h"
#include "Misc/FileHelper.h"
#include "Console.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

TArray<CModule*> CModuleManager::modules;
TArray<FLibrary*> CModuleManager::libraries;

FClass* CModuleManager::FindClass(const FString& name)
{
for (auto* m : modules)
	{
		for (auto* c : m->Classes)
			if (c->GetInternalName() == name)
				return c;
	}

	return nullptr;
}

FStruct* CModuleManager::FindStruct(const FString& name)
{
for (auto* m : modules)
	{
		for (auto* c : m->Structures)
			if (c->GetInternalName() == name)
				return c;
	}
	return nullptr;
}

FEnum* CModuleManager::FindEnum(const FString& name)
{
for (auto* m : modules)
	{
		for (auto* c : m->Enums)
			if (c->GetInternalName() == name)
				return c;
	}
	return nullptr;
}

void CModuleManager::GetAssetTypes(TArray<FAssetClass*>& out)
{
	for (auto* m : modules)
	{
		for (auto* c : m->Assets)
			out.Add(c);
	}
}

void CModuleManager::FindChildClasses(FClass* target, TArray<FClass*>& out)
{
	for (auto* m : modules)
	{
		for (auto c : m->Classes)
		{
			if (c->GetBaseClass() == target)
				out.Add(c);
		}
	}
}

void CModuleManager::GetClassesOfType(FClass* type, TArray<FClass*>& out)
{
	for (auto* m : modules)
	{
		for (auto* c : m->Classes)
		{
			if (c->CanCast(type))
				out.Add(c);
		}
	}
}

int CModuleManager::LoadModule(const FString& path, CModule** outPtr)
{
	//FString path = name + "\\bin\\" + name + ".dll";
	if (!FFileHelper::FileExists(path))
		return 1;

	CModule* m = nullptr;
	typedef CModule* (*GetModule_Func)();

#ifdef PLATFORM_WINDOWS
	HMODULE wModule = LoadLibrary(path.c_str());
	if (!wModule)
		return 2;

	GetModule_Func f = (GetModule_Func)GetProcAddress(wModule, "__GetModuleInstance");
	if (!f)
	{
		DWORD err = GetLastError();
		THORIUM_ASSERT(0, FString("Failed to find proc '__GetModuleInstance'\nerr: ") + FString::ToString(err));
	}
	m = f();
	if (!m)
		return 3;

	m->handle = wModule;
#else
	void* wModule = dlopen(path.c_str(), RTLD_NOW);
	if (!wModule)
		return 2;

	GetModule_Func f = (GetModule_Func)dlsym(wModule, "__GetModuleInstance");
	if (!f)
	{
		THORIUM_ASSERT(0, "Failed to find func '__GetModuleInstance' in library '" + path.c_str() + "'!");
	}

	m = f();
	if (!m)
		return 3;

	m->handle = wModule;
#endif

	if (outPtr)
		*outPtr = m;

	m->path = path;
	modules.Add(m);
	return 0;
}

bool CModuleManager::UnloadModule(const FString& name)
{
	if (name == "Engine")
		return false;

	TIterator<CModule*> it = modules.begin();
	for (; it != modules.end(); it++)
	{
		if (it->Name() == name)
			break;
	}

	if (it == modules.end())
		return false;

	modules.Erase(it);

#ifdef PLATFORM_WINDOWS
	FreeLibrary((HMODULE)it->handle);
#else
	dlclose(it->handle);
#endif

	return true;
}

FLibrary* CModuleManager::LoadFLibrary(const FString& name, const FString& path)
{
	// Check if this library is already loaded
	for (auto* l : libraries)
	{
		if (l->Path() == path)
			return nullptr;
	}

	FLibrary* lib = new FLibrary();
	lib->name = name;
	lib->path = path;

#ifdef PLATFORM_WINDOWS
	HMODULE wModule = LoadLibrary(path.c_str());
	if (!wModule)
	{
		delete lib;
		return nullptr;
	}

	lib->handle = (void*)wModule;
#else
	void* wModule = dlopen(path.c_str(), RTLD_NOW);
	if (!wModule)
	{
		delete lib;
		return nullptr;
	}

	lib->handle = wModule;
#endif

	libraries.Add(lib);
	return lib;
}

bool CModuleManager::UnloadLibrary(FLibrary* lib)
{
	auto it = libraries.Find(lib);
	if (it != libraries.end())
	{
		libraries.Erase(it);
		return true;
	}
	return false;
}

bool CModuleManager::UnloadLibrary(const FString& name)
{
	for (auto* l : libraries)
	{
		if (l->Name() == name)
			return UnloadLibrary(l);
	}
	return false;
}

void CModuleManager::Cleanup()
{
	for (auto m : modules)
	{
		if (m->handle)
		{
#ifdef PLATFORM_WINDOWS
			FreeLibrary((HMODULE)m->handle);
#else
			dlclose(m->handle);
#endif
		}
	}

	for (auto* l : libraries)
	{
		if (l->handle)
		{
#ifdef PLATFORM_WINDOWS
			FreeLibrary((HMODULE)l->handle);
#else
			dlclose(l->handle);
#endif
			delete l;
		}
	}
}

CModule* CModuleManager::FindModule(const FString& name)
{
	for (auto m : modules)
		if (m->Name() == name)
			return m;

	return nullptr;
}

FLibrary::FuncAdress FLibrary::GetFunctionPtr(const FString& fun)
{
	SizeType errCode = 0;
#ifdef PLATFORM_WINDOWS
	auto f = GetProcAddress((HMODULE)handle, fun.c_str());
	errCode = GetLastError();
#else
	void* f = dlsym(handle, fun.c_str());
#endif
	if (!f)
		CONSOLE_LogError("FLibrary", "Unable to obtain FuntionPtr '" + fun + "' from library '" + name + "', error code: " + FString::ToString(errCode));

	return f;
}
