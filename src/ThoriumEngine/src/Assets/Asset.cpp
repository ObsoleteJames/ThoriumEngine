
#include "Asset.h"
#include "Math/Math.h"
#include "Console.h"

void CAsset::Serialize(FMemStream& out)
{
	BaseClass::Serialize(out);
}

bool CAsset::Init()
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CAsset", "Failed to create file stream for '" + file->Path() + "'!");
		return false;
	}

	FAssetHeader info;
	*stream >> &info;

	checksum = info.checksum;
	assetId = info.assetId;
	version = info.fileVersion;
	assetVersion = info.assetVersion;

	OnInit(stream);

	bInitialized = true;
	return true;
}

void CAsset::Save()
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("wb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CAsset", "Failed to create file stream for '" + file->Path() + "'!");
		return;
	}

	FAssetHeader info{};
	info.checksum = -1;
	info.assetVersion = CASSET_VERSION;
	info.fileVersion = GetFileVersion();
	info.assetId = assetId;
	memcpy(info.typeName, GetClass()->cppName.c_str(), FMath::Min(GetClass()->cppName.Size() + 1, 31ull));

	*stream << &info;

	OnSave(stream);

	// TODO: calculate checksum.
}

void CAsset::Load(uint8 lodLevel)
{
	if (IsLoaded(lodLevel))
		return;

	if (!file)
	{
		CONSOLE_LogError("CAsset", "Attempted to load from file, but file isn't present!");
		return;
	}

	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CAsset", "Failed to create file stream for '" + file->Path() + "'!");
		return;
	}

	stream->Seek(sizeof(FAssetHeader), SEEK_SET);

	OnLoad(stream, lodLevel);
}

void CAsset::Load(FMemStream& in)
{
	BaseClass::Load(in);
}

bool CAsset::IsLoaded(uint8 lod) const
{
	return IsLodLoaded(lod);
}

void CAsset::OnDelete()
{
	if (bRegistered)
		CAssetManager::OnAssetDeleted(this);
}

void CAsset::SetLodLevel(uint8 level, bool value)
{
	lodLevels = value ? (lodLevels | (1 << level)) : (lodLevels & ~(1 << level));
}
