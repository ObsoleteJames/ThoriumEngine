
#include "Module.h"
#include "Object/Class.h"
#include "Misc/FileHelper.h"
#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

TArray<CModule*> CModuleManager::modules;

FClass* CModuleManager::FindClass(const FString& name)
{
	//FString moduleName = name;
	//FString className = name;

	//SizeType p = name.FindFirstOf(':');
	//if (p == FString::npos)
	//	return nullptr;

	//moduleName.Erase(moduleName.begin() + p, moduleName.end());
	//className.Erase(moduleName.begin(), moduleName.begin() + p + 1);

	//CModule* _module = FindModule(moduleName);
	//if (!_module)
	//	return nullptr;

	//for (auto c : _module->Classes)
	//{
	//	if (c->GetInternalName() == name)
	//		return c;
	//}

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
	//	FString moduleName = name;
	//	FString className = name;
	//
	//	SizeType p = name.FindFirstOf(':');
	//	if (p == FString::npos)
	//	{
	//		for (auto m : modules)
	//		{
	//			for (auto c : m->Structures)
	//			{
	//				if (c->GetInternalName() == name)
	//					return c;
	//			}
	//		}
	//		return nullptr;
	//	}
	//
	//	moduleName.Erase(moduleName.begin() + p, moduleName.end());
	//	className.Erase(moduleName.begin(), moduleName.begin() + p + 1);
	//
	//	CModule* _module = FindModule(moduleName);
	//	if (!_module)
	//		return nullptr;
	//
	//	for (auto c : _module->Structures)
	//	{
	//		if (c->GetInternalName() == name)
	//			return c;
	//	}

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
	//FString moduleName = name;
	//FString className = name;

	//SizeType p = name.FindFirstOf(':');
	//if (p == FString::npos)
	//	return nullptr;

	//moduleName.Erase(moduleName.begin() + p, moduleName.end());
	//className.Erase(moduleName.begin(), moduleName.begin() + p + 1);

	//CModule* _module = FindModule(moduleName);
	//if (!_module)
	//	return nullptr;

	//for (auto c : _module->Enums)
	//{
	//	if (c->GetInternalName() == name)
	//		return c;
	//}
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

int CModuleManager::LoadModule(const WString& path)
{
	//FString path = name + "\\bin\\" + name + ".dll";
	if (!FFileHelper::FileExists(path))
		return 1;

	CModule* m = nullptr;
	typedef CModule* (*GetModule_Func)();

#ifdef PLATFORM_WINDOWS
	HMODULE wModule = LoadLibraryW(path.c_str());
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

	m->moduleHandle = wModule;
#endif

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
	FreeLibrary((HMODULE)it->moduleHandle);
#endif

	return true;
}

void CModuleManager::Cleanup()
{
	for (auto m : modules)
	{
		if (m->moduleHandle)
		{
#ifdef PLATFORM_WINDOWS
			FreeLibrary((HMODULE)m->moduleHandle);
#endif
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
