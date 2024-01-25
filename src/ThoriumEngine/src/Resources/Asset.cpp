
#include "Asset.h"

void CAsset::SaveAs(const FString& newPath)
{
	FMod* mod = file->Mod();
	file = mod->CreateFile(newPath);

	Save();
}

void CAsset::Serialize(FMemStream& out)
{
	BaseClass::Serialize(out);
}

void CAsset::Load(FMemStream& in)
{
	BaseClass::Load(in);
}

void CAsset::OnDelete()
{
	if (bRegistered)
		CResourceManager::OnResourceDeleted(this);
}

void CAsset::SetLodLevel(uint8 level, bool value)
{
	lodLevels = value ? (lodLevels | (1 << level)) : (lodLevels & ~(1 << level));
}
