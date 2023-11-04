#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "ResourceManager.h"
#include "Asset.generated.h"

ASSET(Abstract)
class ENGINE_API CAsset : public CObject
{
	GENERATED_BODY()

	friend class CResourceManager;

public:
	CAsset() = default;
	virtual ~CAsset() = default;

	inline WString GetPath() const { if (file) return file->Path(); return L""; }
	inline FFile* File() const { return file; }

	virtual void Init() = 0;

	virtual void Save() {}
	virtual void SaveAs(const WString& newPath);

	virtual void Load(uint8 lodLevel) = 0;
	virtual void Unload(uint8 lodLevel) {}

	virtual bool Import(const WString& file) { return false; }

public:
	inline bool IsLodLoaded(uint8 lod);

public:
	void Serialize(FMemStream& out) final {}
	void Load(FMemStream& in) final {}

protected:
	virtual void OnDelete() override;

	void SetLodLevel(uint8 level, bool value);

protected:
	bool bInitialized : 1;
	bool bRegistered : 1;
	
	uint8 lodLevels = 0;
	FFile* file;

};

bool CAsset::IsLodLoaded(uint8 lod)
{
	return lodLevels & (1 << (lod));
}
