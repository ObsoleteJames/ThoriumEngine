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

struct FAssetDependancy
{
	SizeType assetId;
	FString property;
};

struct FAssetData
{
	FFile* file;
	SizeType id;
	uint8 version;
	FAssetClass* type;
};

class IAssetStreamingProxy
{
public:
	virtual ~IAssetStreamingProxy() = default;

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

class ENGINE_API CAssetManager
{
	friend class CAsset;
	friend struct FFile;
	friend struct FMod;
	friend struct FDirectory;

public:
	static void Init();
	static void Shutdown();
	static void Update();

	static void ScanMod(FMod* mod);
	static void DeleteAssetsFromMod(FMod* mod);

	// Converts file into generic asset type
	static void ConvertToAsset(FFile* file, FAssetClass* type);
	
	/**
	 * Registers a dependancy from 'source' to 'target'.
	 * with property being the property on a that depends on B
	 */
	static void RegisterAssetDependancy(SizeType sourceId, SizeType targetId, const FString& property);

	/**
	 * Clears any registered dependancies from 'source' to 'target'.
	 */
	static void ClearAllDependancies(SizeType sourceId, SizeType targetId);
	static void ClearDependancy(SizeType sourceId, SizeType targetId, const FString& property);

	//template<typename T>
	//static inline TObjectPtr<T> GetResource(const FString& path, uint8 lod = 0) { return (TObjectPtr<T>)GetResource((FAssetClass*)T::StaticClass(), path, lod); }

	template<class T>
	inline static TObjectPtr<T> GetAsset(const FString& path) { return (TObjectPtr<T>)CAssetManager::GetAsset((FAssetClass*)T::StaticClass(), path); }

	static TObjectPtr<CAsset> GetAsset(FAssetClass* type, const FString& path);
	static TObjectPtr<CAsset> GetAsset(FAssetClass* type, SizeType assetId);
	
	//template<class T>
	//static void GetAssets(TArray<TObjectPtr<T>>& out);

	inline static const TUnorderedMap<SizeType, FAssetData>& GetAssetsData() { return availableAssets; }
	static const FAssetData* GetAssetData(SizeType assetId) { if (auto it = availableAssets.find(assetId); it != availableAssets.end()) return &it->second; return nullptr; }
	inline static const FAssetData* GetAssetData(const FString& path) { return GetAssetData(GetAssetId(path)); }

	template<class T>
	inline static TObjectPtr<T> CreateAsset(const FString& path, const FString& mod = FString()) { return (TObjectPtr<T>)CreateAsset((FAssetClass*)T::StaticClass(), path, mod); }

	static TObjectPtr<CAsset> CreateAsset(FAssetClass* type, const FString& path, const FString& mod = FString());

	template<class T>
	inline static void LoadAssets() { LoadAssets((FAssetClass*)T::StaticClass()); }

	inline static FAssetClass* GetAssetTypeByFile(FFile* file) { auto it = assetPaths.find(file->Path()); if (it != assetPaths.end()) return GetAssetData(it->second)->type; return nullptr; }
	inline static SizeType GetAssetId(const FString& path) { if (auto it = assetPaths.find(path); it != assetPaths.end()) return it->second; return -1; }

	/**
	 *	Loads all resources of the specified type.
	 */
	static void LoadAssets(FAssetClass* type);

	/**
	 *	Register a user created asset to the resource system.
	 */
	static bool RegisterNewAsset(CAsset* asset, const FString& path, const FString& mod = FString());

	static void StreamAsset(IAssetStreamingProxy* proxy);

	inline static SizeType AssetsCount() { return allocatedAssets.size(); }
	inline static SizeType StreamingAssetsCount() { return streamingAssets.Size(); }

private:
	static CAsset* AllocateAsset(FAssetClass* type, SizeType id);
	static int ScanDir(FDirectory* dir);
	static void OnAssetDeleted(CAsset* asset);

	static void OnAssetFileMoved(FFile* file);
	static void OnAssetFileDeleted(FFile* file);

	static void StreamAssets();

	static bool FetchAssetData(FFile* file, FAssetData& outData);

	static void LoadGenericAssets(FMod* mod);
	static void SaveAssetListBin(FMod* mod);

private:
	static TUnorderedMap<SizeType, CAsset*> allocatedAssets;
	static TUnorderedMap<SizeType, FAssetData> availableAssets;

	static TUnorderedMap<SizeType, FAssetData> genericAssets;

	// map of asset paths and their IDs
	static TUnorderedMap<FString, SizeType> assetPaths;

	static TArray<IAssetStreamingProxy*> streamingAssets;
};

// template<typename T>
// void CAssetManager::GetAssets(TArray<TObjectPtr<T>>& out)
// {
// 	FClass* c = T::StaticClass();
// 	for (auto it : availableAssets)
// 	{
// 		if (it.second.type == c)
// 		{
// 			auto obj = allocatedAssets.find(it.second.file->Path());
// 			if (obj == allocatedAssets.end())
// 			{
// 				CAsset* asset = AllocateAsset((FAssetClass*)c, it.second.file->Path());
// 				asset->file = it.second.file;
// 				asset->Init();
// 				out.Add((T*)asset);
// 			}
// 			else
// 				out.Add((T*)obj->second);
// 		}
// 	}
// }
