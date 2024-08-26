
#include <string>
#include "Engine.h"
#include "AssetManager.h"
#include "Registry/FileSystem.h"
#include "Console.h"
#include "Module.h"
#include "Asset.h"

#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#define RESOURCE_THREAD_COUNT 2

TUnorderedMap<SizeType, CAsset*> CAssetManager::allocatedAssets;
TUnorderedMap<SizeType, FAssetData> CAssetManager::availableAssets;

TUnorderedMap<FString, SizeType> CAssetManager::assetPaths;

TArray<IAssetStreamingProxy*> CAssetManager::streamingAssets;

static std::mutex resourceMutex;
static std::thread resourceThread;
static std::atomic<bool> bResourceRunning;

static TArray<std::thread> resourceThreads;

static CConCmd cmdPrintStreamCount("resources.printinfo", []() { CONSOLE_LogInfo("CAssetManager", "Assets: " + FString::ToString(CAssetManager::AssetsCount()) + "\nStreaming: " + FString::ToString(CAssetManager::StreamingAssetsCount())); });

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

int CAssetManager::ScanDir(FDirectory* dir)
{
	int numFiles = 0;
	for (auto f : dir->files)
	{
		const FString& ext = f->Extension();
		/*FAssetClass* type = GetClassFromExt(ext);
		if (type == nullptr)
			continue;

		availableResources[f->Path()] = { f, type };*/

		if (ext == ".thasset")
		{
			FAssetData data;
			if (!FetchAssetData(f, data))
			{
				// only print the error if the file was actually read.
				if (data.file == f)
					CONSOLE_LogError("CAssetManager", "Failed to get asset data. file is either corrupted or the wrong format!\n" + f->Path());
				continue;
			}

			if (auto it = availableAssets.find(data.id); it != availableAssets.end())
			{
				CONSOLE_LogError("CAssetManager", "Found mutliple assets with same ID!\n" + it->second.file->Path() + '\n' + f->Path() + " - ignored");
				continue;
			}

			assetPaths[f->Path()] = data.id;
			availableAssets[data.id] = data;
			numFiles++;
		}
	}

	for (auto& d : dir->directories)
		numFiles += ScanDir(d);

	return numFiles;
}

void CAssetManager::OnAssetDeleted(CAsset* asset)
{
	auto it = allocatedAssets.find(asset->AssetId());
	if (it == allocatedAssets.end())
		return;

	allocatedAssets.erase(it);
}

void CAssetManager::OnAssetFileMoved(FFile* file)
{
	FString oldPath;
	SizeType id;
	for (auto& r : assetPaths)
	{
		const FAssetData* data = GetAssetData(r.second);
		if (data->file == file)
		{
			id = data->id;
			oldPath = r.first;
			break;
		}
	}

	if (oldPath.IsEmpty())
		return;

	assetPaths.erase(oldPath);
	assetPaths[file->Path()] = id;
	//auto data = availableResources[oldPath];
	//availableResources.erase(oldPath);
	//availableResources[file->Path()] = data;

	//if (auto it = allocatedResources.find(oldPath); it != allocatedResources.end())
	//{
	//	CAsset* asset = it->second;
	//	allocatedResources.erase(it);
	//	allocatedResources[file->Path()] = asset;
	//}
}

void CAssetManager::OnAssetFileDeleted(FFile* file)
{
	auto* data = GetAssetData(file->Path());
	if (!data)
		return;

	if (auto it = allocatedAssets.find(data->id); it != allocatedAssets.end())
	{
		it->second->Delete();
		allocatedAssets.erase(data->id);
	}

	/*if (auto it = allocatedResources.find(file->Path()); it != allocatedResources.end())
	{
		it->second->Delete();
		allocatedResources.erase(file->Path());
	}*/

	if (availableAssets.find(data->id) != availableAssets.end())
		availableAssets.erase(data->id);

	if (assetPaths.find(file->Path()) != assetPaths.end())
		assetPaths.erase(file->Path());
}

void CAssetManager::Init()
{
	bResourceRunning = true;
	//resourceThread = std::thread(&CResourceManager::StreamResources);
	resourceThreads.Resize(RESOURCE_THREAD_COUNT);
	for (int i = 0; i < RESOURCE_THREAD_COUNT; i++)
		resourceThreads[i] = std::thread(&CAssetManager::StreamAssets);

	CONSOLE_LogInfo("CAssetManager", "Created asset streaming threads (" + FString::ToString(RESOURCE_THREAD_COUNT) + ")");
	CONSOLE_LogInfo("CAssetManager", "Initialized");
}

void CAssetManager::Shutdown()
{
	bResourceRunning = false;
	CONSOLE_LogInfo("CAssetManager", "Shutting down asset streaming threads...");
	for (int i = 0; i < RESOURCE_THREAD_COUNT; i++)
		resourceThreads[i].join();

	resourceThreads.Clear();
}

void CAssetManager::Update()
{
	resourceMutex.lock();
	if (streamingAssets.Size() == 0)
	{
		resourceMutex.unlock();
		return;
	}

	for (int i = 0; i < streamingAssets.Size(); i++)
	{
		IAssetStreamingProxy* obj = streamingAssets[i];
		if (obj->bDirty)
			obj->PushData();

		if (obj->bFinished && !obj->bLoading)
		{
			streamingAssets.Erase(streamingAssets.begin() + i);
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

void CAssetManager::StreamAssets()
{
	using namespace std::chrono_literals;
	int index = 0;
	while (bResourceRunning)
	{
		resourceMutex.lock();
		if (streamingAssets.Size() == 0)
		{
			resourceMutex.unlock();
			std::this_thread::sleep_for(1ms);
			continue;
		}

		IAssetStreamingProxy* obj = streamingAssets[index];
		index++;
		index %= streamingAssets.Size();
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

bool CAssetManager::FetchAssetData(FFile* file, FAssetData& outData)
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CAssetManager", "Failed to create file stream for '" + file->Path() + "'!");
		return false;
	}

	FAssetHeader info;
	*stream >> &info;
	info.typeName[31] = '\0'; // just in case this file is corrupted or the wrong format.

	FString typeName = info.typeName;

	outData.file = file;
	outData.id = info.assetId;
	outData.type = (FAssetClass*)CModuleManager::FindClass(typeName);
	outData.version = info.assetVersion;

	return outData.type != nullptr;
}

void CAssetManager::ScanMod(FMod* mod)
{
	int numFiles = ScanDir(&mod->root);

	CONSOLE_LogInfo("CAssetManager", FString("Found ") + std::to_string(numFiles).c_str() + " assets in '" + mod->Name() + "'");

	for (auto& it : availableAssets)
	{
		FAssetClass* Class = it.second.type;
		if (Class->AssetFlags() & ASSET_AUTO_LOAD)
		{
			// Check if this resource is not already loaded
			auto r = allocatedAssets.find(it.first);
			if (r == allocatedAssets.end())
			{
				CAsset* asset = AllocateAsset(Class, it.first);
				asset->file = it.second.file;
				asset->SetName(asset->file->Name() + asset->file->Extension());
				asset->Init();
			}
		}
	}
}

void CAssetManager::DeleteAssetsFromMod(FMod* mod)
{
	auto ar = availableAssets;

	for (auto& it : ar)
	{
		if (it.second.file->Mod() == mod)
		{
			auto allocated = allocatedAssets.find(it.first);
			if (allocated != allocatedAssets.end())
				allocated->second->Delete();
			
			availableAssets.erase(it.first);
		}
	}
}

void CAssetManager::RegisterAssetDependancy(SizeType idA, SizeType idB, const FString& property)
{

}

void CAssetManager::ClearAllDependancies(SizeType idA, SizeType idB)
{

}

void CAssetManager::ClearDependancy(SizeType idA, SizeType idB, const FString& property)
{

}

TObjectPtr<CAsset> CAssetManager::GetAsset(FAssetClass* type, const FString& path)
{
	if (path.IsEmpty())
		return nullptr;

	CAsset* r = GetAsset(type, GetAssetId(path));
	if (!r)
		CONSOLE_LogError("CAssetManager", "Failed to get resource '" + path + "'!");
	return r;
}

TObjectPtr<CAsset> CAssetManager::GetAsset(FAssetClass* type, SizeType assetId)
{
	CAsset* asset = nullptr;
	auto it = allocatedAssets.find(assetId);
	if (it == allocatedAssets.end())
	{
		auto file = availableAssets.find(assetId);
		if (file == availableAssets.end())
		{
			//CONSOLE_LogError("CResourceManager", "Failed to get resource '" + path + "', resource doesn't exist!");
			return nullptr;
		}

		if (type && file->second.type != type)
		{
			CONSOLE_LogError("CAssetManager", "Failed to get asset '" + FString::ToString(assetId) + "', Invalid Type! expected " + type->GetName());
			return nullptr;
		}

		asset = AllocateAsset(file->second.type, assetId);
		asset->file = file->second.file;
		asset->SetName(asset->file->Name() + asset->file->Extension());
		asset->Init();
	}
	else
	{
		asset = it->second;
		if ((FAssetClass*)asset->GetClass() != type)
		{
			CONSOLE_LogError("CAssetManager", "Failed to get asset '" + FString::ToString(assetId) + "', Invalid Type! expected " + type->GetName());
			return nullptr;
		}
	}

	return asset;
}

TObjectPtr<CAsset> CAssetManager::CreateAsset(FAssetClass* type, const FString& p, const FString& m /*= L""*/)
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
	FString ext = ".thasset";
	path = path + ext;

	int numCopies = 0;
	while (FFile* f = mod->FindFile(path))
	{
		path = fileNoExt + "_" + FString::ToString(numCopies) + ext;
		numCopies++;
	}

	FFile* file = mod->CreateFile(path);

	SizeType assetId = FMath::Random64();

	availableAssets[assetId] = { file, assetId, CASSET_VERSION, type };

	assetPaths[file->Path()] = assetId;
	
	CAsset* asset = AllocateAsset(type, assetId);
	asset->file = file;
	asset->SetName(asset->file->Name());
	//asset->Init();
	return asset;
}

void CAssetManager::LoadAssets(FAssetClass* type)
{
	for (auto it : availableAssets)
	{
		if (it.second.type == type)
		{
			auto obj = allocatedAssets.find(it.first);
			if (obj == allocatedAssets.end())
			{
				CAsset* asset = AllocateAsset(type, it.first);
				asset->file = it.second.file;
				asset->SetName(asset->file->Name());
				asset->Init();
			}
		}
	}
}

bool CAssetManager::RegisterNewAsset(CAsset* asset, const FString& p, const FString& m /*= L""*/)
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
	path += ".thasset";

	SizeType assetId = FMath::Random64();
	asset->assetId = assetId;
	asset->assetVersion = CASSET_VERSION;

	FFile* file = mod->CreateFile(path);
	availableAssets[assetId] = { file, assetId, CASSET_VERSION, (FAssetClass*)asset->GetClass() };

	assetPaths[file->Path()] = assetId;

	asset->file = file;
	allocatedAssets[assetId] = asset;
	asset->bRegistered = true;
	return true;
}

void CAssetManager::StreamAsset(IAssetStreamingProxy* proxy)
{
	resourceMutex.lock();
	streamingAssets.Add(proxy);
	resourceMutex.unlock();
}

CAsset* CAssetManager::AllocateAsset(FAssetClass* type, SizeType id)
{
	CAsset* r = (CAsset*)type->Instantiate();
	allocatedAssets[id] = r;
	r->assetId = id;
	r->bRegistered = true;
	return r;
}
