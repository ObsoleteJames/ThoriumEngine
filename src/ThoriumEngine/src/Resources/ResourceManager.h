#pragma once

#include <string>
#include <mutex>
#include <Util/FStream.h>
#include "EngineCore.h"
#include "Object/Object.h"
#include "Registry/FileSystem.h"

#include <Util/Map.h>

//#include "ResourceManager.generated.h"

class FAssetClass;
class CAsset;

struct FResourceData
{
	FFile* file;
	FAssetClass* type;
};

class IResourceStreamingProxy
{
public:
	virtual ~IResourceStreamingProxy() = default;

	virtual void Load() = 0;
	virtual void PushData() = 0;

public:
	std::mutex loadLock;

	// whether this resource is currently loading.
	std::atomic<bool> bLoading;
	// whether this resource has finished loading.
	std::atomic<bool> bFinished;
	// whether this object has new data to pushed to the resource.
	std::atomic<bool> bDirty;
};

class ENGINE_API CResourceManager
{
	friend class CAsset;
	friend struct FFile;

public:
	static void Init();
	static void Shutdown();
	static void Update();

	static void ScanMod(FMod* mod);
	static void RegisterNewFile(FFile* file);
	
	//template<typename T>
	//static inline TObjectPtr<T> GetResource(const FString& path, uint8 lod = 0) { return (TObjectPtr<T>)GetResource((FAssetClass*)T::StaticClass(), path, lod); }

	template<class T>
	inline static TObjectPtr<T> GetResource(const FString& path) { return (TObjectPtr<T>)CResourceManager::GetResource((FAssetClass*)T::StaticClass(), path); }

	static TObjectPtr<CAsset> GetResource(FAssetClass* type, const FString& path);
	
	template<class T>
	static void GetResources(TArray<TObjectPtr<T>>& out);

	static const TUnorderedMap<FString, FResourceData>& GetAvailableResources() { return availableResources; }

	template<class T>
	inline static TObjectPtr<T> CreateResource(const FString& path, const FString& mod = FString()) { return (TObjectPtr<T>)CreateResource((FAssetClass*)T::StaticClass(), path, mod); }

	static TObjectPtr<CAsset> CreateResource(FAssetClass* type, const FString& path, const FString& mod = FString());

	template<class T>
	inline static void LoadResources() { LoadResources((FAssetClass*)T::StaticClass()); }

	inline static FAssetClass* GetResourceTypeByFile(FFile* file) { auto it = availableResources.find(file->Path()); if (it != availableResources.end()) return it->second.type; return nullptr; }

	/**
	 *	Loads all resources of the specified type.
	 */
	static void LoadResources(FAssetClass* type);

	/**
	 *	Register a user created asset to the resource system.
	 */
	static bool RegisterNewResource(CAsset* resource, const FString& path, const FString& mod = L"");

	static void StreamResource(IResourceStreamingProxy* proxy);

	inline static SizeType ResourcesCount() { return allocatedResources.size(); }
	inline static SizeType StreamingResourcesCount() { return streamingResources.Size(); }

private:
	static CAsset* AllocateResource(FAssetClass* type, const FString& path);
	static int ScanDir(FDirectory* dir);
	static void OnResourceDeleted(CAsset* asset);

	static void OnResourceFileDeleted(FFile* file);

	static void StreamResources();

private:
	static TUnorderedMap<FString, CAsset*> allocatedResources;
	static TUnorderedMap<FString, FResourceData> availableResources;
	static TArray<IResourceStreamingProxy*> streamingResources;
};

template<typename T>
void CResourceManager::GetResources(TArray<TObjectPtr<T>>& out)
{
	FClass* c = T::StaticClass();
	for (auto it : availableResources)
	{
		if (it.second.type == c)
		{
			auto obj = allocatedResources.find(it.second.file->Path());
			if (obj == allocatedResources.end())
			{
				CAsset* asset = AllocateResource((FAssetClass*)c, it.second.file->Path());
				asset->file = it.second.file;
				asset->Init();
				out.Add((T*)asset);
			}
			else
				out.Add((T*)obj->second);
		}
	}
}
