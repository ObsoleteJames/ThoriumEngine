
#include <Util/Assert.h>
#include "Module.h"
#include "Misc/FileHelper.h"
#include "Console.h"

#include "unistd.h"
#include "dlfcn.h"

int CModuleManager::LoadModule(const FString& path, CModule** outPtr)
{
	//FString path = name + "\\bin\\" + name + ".dll";
	if (!FFileHelper::FileExists(path))
		return 1;

	CModule* m = nullptr;
	typedef CModule* (*GetModule_Func)();

	void* wModule = dlopen(path.c_str(), RTLD_NOW);
	if (!wModule)
		return 2;

	GetModule_Func f = (GetModule_Func)dlsym(wModule, "__GetModuleInstance");
	if (!f)
	{
		THORIUM_ASSERT(0, FString("Failed to find func '__GetModuleInstance' in library '") + path.c_str() + "'!");
	}

	m = f();
	if (!m)
		return 3;

	m->handle = wModule;

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

	dlclose(it->handle);
	return true;
}

bool CModuleManager::UnloadModule(CModule* module)
{
	if (module->name == "Engine")
		return false;

	dlclose(module->handle);

	modules.Erase(modules.Find(module));
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

	void* wModule = dlopen(path.c_str(), RTLD_NOW);
	if (!wModule)
	{
		delete lib;
		return nullptr;
	}

	lib->handle = wModule;

	libraries.Add(lib);
	return lib;
}

void CModuleManager::Cleanup()
{
	for (auto m : modules)
	{
		if (m->handle)
		{
			dlclose(m->handle);
		}
	}

	for (auto* l : libraries)
	{
		if (l->handle)
		{
			dlclose(l->handle);
			delete l;
		}
	}
}

FLibrary::FuncAdress FLibrary::GetFunctionPtr(const FString& fun)
{
	SizeType errCode = 0;

	void* f = dlsym(handle, fun.c_str());
    
	if (!f)
		CONSOLE_LogError("FLibrary", "Unable to obtain FuntionPtr '" + fun + "' from library '" + name + "', error code: " + FString::ToString(errCode));

	return f;
}
