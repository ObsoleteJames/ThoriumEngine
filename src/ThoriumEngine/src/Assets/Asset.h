#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "AssetManager.h"
#include "Asset.generated.h"

#define CASSET_VERSION 0x01

struct FAssetHeader
{
	SizeType checksum;
	SizeType assetId;
	uint8 assetVersion;
	uint8 fileVersion;
	char typeName[32];
};

ASSET(Abstract)
class ENGINE_API CAsset : public CObject
{
	GENERATED_BODY()

	friend class CAssetManager;

public:
	CAsset() = default;
	virtual ~CAsset() = default;

	bool Init();

	void Save();
	//void SaveAs(const FString& newPath); // Replaced by CAssetManager::DupeAsset(SizeType id, const FString& newPath)

	void Load(uint8 lodLevel);
	virtual void Unload(uint8 lodLevel) {}

public:
	inline bool IsLodLoaded(uint8 lod);
	
	inline SizeType AssetId() const { return assetId; }

	inline FString GetPath() const { if (file) return file->Path(); return ""; }
	inline FFile* File() const { return file; }

public:
	void Serialize(FMemStream& out) final;
	void Load(FMemStream& in) final;

protected:
	virtual void OnInit(IBaseFStream* stream) {}
	virtual void OnSave(IBaseFStream* stream) {}
	virtual void OnLoad(IBaseFStream* stream, uint8 lodLevel) = 0;

	virtual uint8 GetFileVersion() const { return -1; }

	virtual void OnDelete() override;

	void SetLodLevel(uint8 level, bool value);

protected:
	bool bInitialized : 1;
	bool bRegistered : 1;
	
	SizeType checksum;
	SizeType assetId;

	uint8 version;
	
	// CAsset version, used incase we need to add more information to asset files later.
	uint8 assetVersion;

	uint8 lodLevels = 0;
	FFile* file;

};

bool CAsset::IsLodLoaded(uint8 lod)
{
	return lodLevels & (1 << (lod));
}
