
#include <string>
#include "Engine.h"
#include "ResourceManager.h"
#include "Registry/FileSystem.h"
#include "Console.h"
#include "Module.h"
#include "Asset.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#define RESOURCE_THREAD_COUNT 2

TUnorderedMap<FString, CAsset*> CResourceManager::allocatedResources;
TUnorderedMap<FString, FResourceData> CResourceManager::availableResources;
TArray<IResourceStreamingProxy*> CResourceManager::streamingResources;

static std::mutex resourceMutex;
static std::thread resourceThread;
static std::atomic<bool> bResourceRunning;

static TArray<std::thread> resourceThreads;

static CConCmd cmdPrintStreamCount("resources.printinfo", []() { CONSOLE_LogInfo("CResourceManager", "Resources: " + FString::ToString(CResourceManager::ResourcesCount()) + "\nStreaming: " + FString::ToString(CResourceManager::StreamingResourcesCount())); });

FAssetClass* GetClassFromExt(const FString& ext)
{
	for (CModule* m : CModuleManager::GetModules())
	{
		for (auto a : m->Assets)
		{
			if (a->GetExtension() == ext)
				return a;
		}
	}
	return nullptr;
}

int CResourceManager::ScanDir(FDirectory* dir)
{
	int numFiles = 0;
	for (auto f : dir->files)
	{
		const FString& ext = f->Extension();
		FAssetClass* type = GetClassFromExt(ext);
		if (type == nullptr)
			continue;

		availableResources[f->Path()] = { f, type };
		numFiles++;
	}

	for (auto& d : dir->directories)
		numFiles += ScanDir(d);

	return numFiles;
}

void CResourceManager::OnResourceDeleted(CAsset* asset)
{
	auto it = allocatedResources.find(asset->GetPath());
	if (it == allocatedResources.end())
		return;

	allocatedResources.erase(it);
}

void CResourceManager::OnResourceFileMoved(FFile* file)
{
	FString oldPath;
	for (auto& r : availableResources)
	{
		if (r.second.file == file)
		{
			oldPath = r.first;
			break;
		}
	}

	if (oldPath.IsEmpty())
		return;

	auto data = availableResources[oldPath];
	availableResources.erase(oldPath);
	availableResources[file->Path()] = data;

	if (auto it = allocatedResources.find(oldPath); it != allocatedResources.end())
	{
		CAsset* asset = it->second;
		allocatedResources.erase(it);
		allocatedResources[file->Path()] = asset;
	}
}

void CResourceManager::OnResourceFileDeleted(FFile* file)
{
	if (auto it = allocatedResources.find(file->Path()); it != allocatedResources.end())
	{
		it->second->Delete();
		allocatedResources.erase(file->Path());
	}

	if (availableResources.find(file->Path()) != availableResources.end())
		availableResources.erase(file->Path());
}

void CResourceManager::Init()
{
	bResourceRunning = true;
	//resourceThread = std::thread(&CResourceManager::StreamResources);
	resourceThreads.Resize(RESOURCE_THREAD_COUNT);
	for (int i = 0; i < RESOURCE_THREAD_COUNT; i++)
		resourceThreads[i] = std::thread(&CResourceManager::StreamResources);

	CONSOLE_LogInfo("CResourceManager", "Created resource threads (" + FString::ToString(RESOURCE_THREAD_COUNT) + ")");
	CONSOLE_LogInfo("CResourceManager", "Initialized");
}

void CResourceManager::Shutdown()
{
	bResourceRunning = false;
	CONSOLE_LogInfo("CResourceManager", "Shutting down resource threads...");
	for (int i = 0; i < RESOURCE_THREAD_COUNT; i++)
		resourceThreads[i].join();

	resourceThreads.Clear();
}

void CResourceManager::Update()
{
	resourceMutex.lock();
	if (streamingResources.Size() == 0)
	{
		resourceMutex.unlock();
		return;
	}

	for (int i = 0; i < streamingResources.Size(); i++)
	{
		IResourceStreamingProxy* obj = streamingResources[i];
		if (obj->bDirty)
			obj->PushData();

		if (obj->bFinished && !obj->bLoading)
		{
			streamingResources.Erase(streamingResources.begin() + i);
			delete obj;
		}
	}

	//IResourceStreamingProxy* obj = streamingResources.first();
	//if (obj->bDirty)
	//	obj->PushData();

	//if (obj->bFinished)
	//{
	//	streamingResources.Erase(streamingResources.first());
	//	delete obj;
	//}

	resourceMutex.unlock();
}

void CResourceManager::StreamResources()
{
	using namespace std::chrono_literals;
	int index = 0;
	while (bResourceRunning)
	{
		resourceMutex.lock();
		if (streamingResources.Size() == 0)
		{
			resourceMutex.unlock();
			std::this_thread::sleep_for(1ms);
			continue;
		}

		IResourceStreamingProxy* obj = streamingResources[index];
		index++;
		index %= streamingResources.Size();
		resourceMutex.unlock();

		if (obj->bFinished)
		{
			//std::this_thread::sleep_for(1ms);
			continue;
		}

		if (!obj->bFinished && !obj->bLoading)
		{
			if (obj->loadLock.try_lock())
			{
				obj->Load();
				obj->loadLock.unlock();
			}
		}
	}
}

void CResourceManager::ScanMod(FMod* mod)
{
	int numFiles = ScanDir(&mod->root);

	CONSOLE_LogInfo("CResourceManager", FString("Found ") + std::to_string(numFiles).c_str() + " assets in '" + mod->Name() + "'");

	for (auto& it : availableResources)
	{
		FAssetClass* Class = it.second.type;
		if (Class->AssetFlags() & ASSET_AUTO_LOAD)
		{
			// Check if this resource is not already loaded
			auto r = allocatedResources.find(it.first);
			if (r == allocatedResources.end())
			{
				CAsset* asset = AllocateResource(Class, it.first);
				asset->file = it.second.file;
				asset->SetName(asset->file->Name() + asset->file->Extension());
				asset->Init();
			}
		}
	}
}

void CResourceManager::DeleteResourcesFromMod(FMod* mod)
{
	auto ar = availableResources;

	for (auto& it : ar)
	{
		if (it.second.file->Mod() == mod)
		{
			auto allocated = allocatedResources.find(it.first);
			if (allocated != allocatedResources.end())
				allocated->second->Delete();
			
			availableResources.erase(it.first);
		}
	}
}

void CResourceManager::RegisterNewFile(FFile* file)
{
	const FString& ext = file->Extension();
	FAssetClass* type = GetClassFromExt(ext);
	if (type == nullptr)
		return;

	availableResources[file->Path()] = { file, type };
}

TObjectPtr<CAsset> CResourceManager::GetResource(FAssetClass* type, const FString& path)
{
	if (path.IsEmpty())
		return nullptr;

	CAsset* asset = nullptr;
	auto it = allocatedResources.find(path);
	if (it == allocatedResources.end())
	{
		auto file = availableResources.find(path);
		if (file == availableResources.end())
		{
			CONSOLE_LogError("CResourceManager", "Failed to get resource '" + path + "', resource doesn't exist!");
			return nullptr;
		}

		if (type && file->second.type != type)
		{
			CONSOLE_LogError("CResourceManager", "Failed to get resource '" + path + "', Invalid Type! expected " + type->GetName());
			return nullptr;
		}

		asset = AllocateResource(file->second.type, path);
		asset->file = file->second.file;
		asset->SetName(asset->file->Name() + asset->file->Extension());
		asset->Init();
	}
	else
	{
		asset = it->second;
		if ((FAssetClass*)asset->GetClass() != type)
		{
			CONSOLE_LogError("CResourceManager", "Failed to get resource '" + path + "', Invalid Type! expected " + type->GetName());
			return nullptr;
		}
	}

	return asset;
}

TObjectPtr<CAsset> CResourceManager::CreateResource(FAssetClass* type, const FString& p, const FString& m /*= L""*/)
{
	FString path = p;
	FString modPath = m;
	if (modPath.IsEmpty())
	{
		SizeType i = p.FindFirstOf(':');
		if (i != -1)
		{
			modPath = p;
			modPath.Erase(modPath.begin() + i, modPath.end());
			path.Erase(path.begin(), path.begin() + i + 2);
		}
		else
			modPath = gEngine->GetProjectConfig().name;
	}
	FMod* mod = CFileSystem::FindMod(modPath);
	
	if (!mod)
		return nullptr;

	if (SizeType i = path.FindLastOf('.'); i != -1)
		path.Erase(path.begin() + i, path.end());

	FString fileNoExt = path;
	FString ext = type->GetExtension();
	path = path + ext;

	int numCopies = 0;
	while (FFile* f = mod->FindFile(path))
	{
		path = fileNoExt + "_" + FString::ToString(numCopies) + ext;
		numCopies++;
	}

	FFile* file = mod->CreateFile(path);

	availableResources[path] = { file, type };
	
	CAsset* asset = AllocateResource(type, path);
	asset->file = file;
	asset->SetName(asset->file->Name() + asset->file->Extension());
	//asset->Init();
	return asset;
}

void CResourceManager::LoadResources(FAssetClass* type)
{
	for (auto it : availableResources)
	{
		if (it.second.type == type)
		{
			auto obj = allocatedResources.find(it.second.file->Path());
			if (obj == allocatedResources.end())
			{
				CAsset* asset = AllocateResource(type, it.second.file->Path());
				asset->file = it.second.file;
				asset->SetName(asset->file->Name() + asset->file->Extension());
				asset->Init();
			}
		}
	}
}

bool CResourceManager::RegisterNewResource(CAsset* resource, const FString& p, const FString& m /*= L""*/)
{
	FString path = p;
	FString modPath = m;
	if (path.IsEmpty())
		return false;

	if (path[0] == '\\' || path[0] == '/')
		path.Erase(path.first());

	if (modPath.IsEmpty())
	{
		SizeType i = p.FindFirstOf(':');
		if (i != -1)
		{
			modPath = p;
			modPath.Erase(modPath.begin() + i, modPath.end());
			path.Erase(path.begin(), path.begin() + i + 2);
		}
		else
			modPath = gEngine->GetProjectConfig().name;
	}
	FMod* mod = CFileSystem::FindMod(modPath);

	if (!mod)
		return false;

	if (SizeType i = path.FindLastOf('.'); i != -1)
		path.Erase(path.begin() + i, path.end());
	path += ((FAssetClass*)resource->GetClass())->GetExtension();

	FFile* file = mod->CreateFile(path);
	availableResources[path] = { file, (FAssetClass*)resource->GetClass() };

	resource->file = file;
	allocatedResources[path] = resource;
	resource->bRegistered = true;
	return true;
}

void CResourceManager::StreamResource(IResourceStreamingProxy* proxy)
{
	resourceMutex.lock();
	streamingResources.Add(proxy);
	resourceMutex.unlock();
}

CAsset* CResourceManager::AllocateResource(FAssetClass* type, const FString& path)
{
	CAsset* r = (CAsset*)type->Instantiate();
	allocatedResources[path] = r;
	r->bRegistered = true;
	return r;
}
