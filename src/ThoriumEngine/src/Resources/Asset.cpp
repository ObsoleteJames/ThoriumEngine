
#include "Asset.h"

void CAsset::SaveAs(const WString& newPath)
{
	FMod* mod = file->Mod();
	file = mod->CreateFile(newPath);

	Save();
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
