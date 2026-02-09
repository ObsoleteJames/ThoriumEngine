
#include <string>
#include "Engine.h"
#include "AssetManager.h"
#include "Registry/FileSystem.h"
#include "Console.h"
#include "Module.h"
#include "Asset.h"

#include <Util/KeyValue.h>

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>
#include <chrono>

#define RESOURCE_THREAD_COUNT 2

TUnorderedMap<SizeType, CAsset*> CAssetManager::allocatedAssets;
TUnorderedMap<SizeType, FAssetData> CAssetManager::availableAssets;

TUnorderedMap<SizeType, FAssetData> CAssetManager::genericAssets;

TUnorderedMap<FString, SizeType> CAssetManager::assetPaths;

TArray<IAssetStreamingProxy*> CAssetManager::streamingAssets;

static std::shared_mutex resourceMutex;
static std::mutex streamMutex;
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
				if (it->second.file != f) // if it's the same file then it's not actually a duplicate
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
	std::unique_lock<std::shared_mutex> lock(resourceMutex);
	auto it = allocatedAssets.find(asset->AssetId());
	if (it == allocatedAssets.end())
		return;

	allocatedAssets.erase(it);
}

void CAssetManager::OnAssetFileMoved(FFile* file)
{
	std::unique_lock<std::shared_mutex> lock(resourceMutex);
	FString oldPath;
	SizeType id;
	for (auto& r : assetPaths)
	{
		const FAssetData* data = GetAssetData(r.second);
		if (data && data->file == file)
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
	std::unique_lock<std::shared_mutex> lock(resourceMutex);
	const FAssetData* data = GetAssetData(file->Path());
	if (!data)
		return;

	SizeType dataId = data->id;

	if (auto it = allocatedAssets.find(dataId); it != allocatedAssets.end())
	{
		it->second->Delete();
		allocatedAssets.erase(dataId);
	}

	if (availableAssets.find(dataId) != availableAssets.end())
		availableAssets.erase(dataId);

	if (assetPaths.find(file->Path()) != assetPaths.end())
		assetPaths.erase(file->Path());

	if (genericAssets.find(dataId) != genericAssets.end())
		genericAssets.erase(dataId);
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
	streamMutex.lock();
	if (streamingAssets.Size() == 0)
	{
		streamMutex.unlock();
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

	streamMutex.unlock();
}

void CAssetManager::StreamAssets()
{
	using namespace std::chrono_literals;
	int index = 0;
	while (bResourceRunning)
	{
		streamMutex.lock();
		if (streamingAssets.Size() == 0)
		{
			streamMutex.unlock();
			std::this_thread::sleep_for(1ms);
			continue;
		}

		IAssetStreamingProxy* obj = streamingAssets[index];
		index++;
		index %= streamingAssets.Size();
		streamMutex.unlock();

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

void CAssetManager::LoadGenericAssets(FMod* mod)
{
	FString binPath = mod->Path() + "/asset_list.bin";
	FKeyValue kv(binPath);
	if (!kv.IsOpen())
		return;

	std::unique_lock<std::shared_mutex> lock(resourceMutex);

	for (auto& v : kv.GetCategories())
	{
		SizeType id = std::stoull(v->GetName().c_str());

		FString path = *v->GetValue("path");
		FString typeStr = *v->GetValue("type");

		FAssetClass* type = (FAssetClass*)CModuleManager::FindClass(typeStr);
		if (!type)
		{
			CONSOLE_LogError("CAssetManager", "Invalid asset type! '" + typeStr + "' does not exist. asset: \"" + path + "\"");
			continue;
		}

		FFile* file = CFileSystem::FindFile(path);
		if (!file)
		{
			CONSOLE_LogError("CAssetManager", "Invalid asset file path ! \"" + path + "\"");
			continue;
		}

		FAssetData data{};
		data.id = id;
		data.file = file;
		data.type = type;
		data.version = CASSET_VERSION_GENERIC_TYPE;

		genericAssets[id] = data;
		availableAssets[id] = data;
		assetPaths[path] = id;
	}
}

void CAssetManager::SaveAssetListBin(FMod* mod)
{
	std::shared_lock<std::shared_mutex> lock(resourceMutex);

	FString binPath = mod->Path() + "/asset_list.bin";
	FKeyValue kv(binPath);

	int numAssets = 0;

	for (auto& asset : genericAssets)
	{
		if (asset.second.file->Mod() != mod)
			continue;

		KVCategory* cat = kv.GetCategory(FString::ToString(asset.second.id), true);
		cat->SetValue("path", asset.second.file->Path());
		cat->SetValue("type", asset.second.type->GetInternalName());

		numAssets++;
	}

	lock.unlock();

	if (numAssets > 0)
		kv.Save();
}

void CAssetManager::ScanMod(FMod* mod)
{
	int numFiles = ScanDir(&mod->root);

	CONSOLE_LogInfo("CAssetManager", FString("Found ") + std::to_string(numFiles).c_str() + " assets in '" + mod->Name() + "'");

	std::unique_lock<std::shared_mutex> lock(resourceMutex);

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
				lock.unlock();
				asset->Init();
				lock.lock();
			}
		}
	}

	lock.unlock();

	LoadGenericAssets(mod);
}

void CAssetManager::DeleteAssetsFromMod(FMod* mod)
{
	std::unique_lock<std::shared_mutex> lock(resourceMutex);

	auto ar = availableAssets;

	lock.unlock();

	SaveAssetListBin(mod);

	lock.lock();

	for (auto& it : ar)
	{
		if (it.second.file->Mod() == mod)
		{
			auto allocated = allocatedAssets.find(it.first);
			if (allocated != allocatedAssets.end())
			{
				lock.unlock();
				allocated->second->Delete();
				lock.lock();
			}
			
			availableAssets.erase(it.first);
		}
	}
}

void CAssetManager::ConvertToAsset(FFile* file, FAssetClass* type)
{
	std::unique_lock<std::shared_mutex> lock(resourceMutex);

	SizeType id = FMath::Random64();

	FAssetData data{};
	data.id = id;
	data.file = file;
	data.type = type;
	data.version = CASSET_VERSION_GENERIC_TYPE;

	genericAssets[id] = data;
	availableAssets[id] = data;
	assetPaths[file->Path()] = id;
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
	std::unique_lock<std::shared_mutex> lock(resourceMutex);

	TObjectPtr<CAsset> asset = nullptr;
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
		
		lock.unlock();
		asset->Init();
		return asset;
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

bool CAssetManager::IsAssetLoaded(SizeType id)
{
	std::shared_lock<std::shared_mutex> lock(resourceMutex);
	auto it = allocatedAssets.find(id);
	return it != allocatedAssets.end();
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

	std::unique_lock<std::shared_mutex> lock(resourceMutex);
	availableAssets[assetId] = { file, assetId, CASSET_VERSION, type };
	assetPaths[file->Path()] = assetId;

	CAsset* asset = AllocateAsset(type, assetId);
	asset->file = file;
	asset->SetName(asset->file->Name());
	lock.unlock();

	return asset;
}

void CAssetManager::LoadAssets(FAssetClass* type)
{
	std::shared_lock<std::shared_mutex> lock(resourceMutex);

	for (auto it : availableAssets)
	{
		if (it.second.type == type)
		{
			auto obj = allocatedAssets.find(it.first);
			if (obj == allocatedAssets.end())
			{
				lock.unlock();
				CAsset* asset = AllocateAsset(type, it.first);
				asset->file = it.second.file;
				asset->SetName(asset->file->Name());
				asset->Init();
				lock.lock();
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

	std::unique_lock<std::shared_mutex> lock(resourceMutex);
	availableAssets[assetId] = { file, assetId, CASSET_VERSION, (FAssetClass*)asset->GetClass() };
	assetPaths[file->Path()] = assetId;
	asset->file = file;
	allocatedAssets[assetId] = asset;
	lock.unlock();

	asset->bRegistered = true;
	return true;
}

void CAssetManager::StreamAsset(IAssetStreamingProxy* proxy)
{
	std::unique_lock<std::shared_mutex> lock(resourceMutex);
	streamingAssets.Add(proxy);
}

CAsset* CAssetManager::AllocateAsset(FAssetClass* type, SizeType id)
{
	CAsset* r = (CAsset*)type->Instantiate();
	allocatedAssets[id] = r;
	
	if (const FAssetData* data = GetAssetData(id); data)
		r->version = data->version;

	r->assetId = id;
	r->bRegistered = true;

	return r;
}
