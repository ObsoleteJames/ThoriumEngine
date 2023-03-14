
#include "BaseImportDialog.h"
#include <Util/Map.h>

static TMap<FAssetClass*, FImportDialogRegister*>& GetImportDialogs()
{
	static TMap<FAssetClass*, FImportDialogRegister*> importDialogs;
	return importDialogs;
}

FImportDialogRegister::FImportDialogRegister(FAssetClass* target, std::function<IImportDialog* (QWidget*)> func) : targetAsset(target), dialogFunc(func)
{
	IImportDialog::RegisterImportDialog(this);
}

void IImportDialog::RegisterImportDialog(FImportDialogRegister* r)
{
	GetImportDialogs()[r->targetAsset] = r;
}

IImportDialog* IImportDialog::GetImportDialog(FAssetClass* Class, QWidget* parent)
{
	auto it = GetImportDialogs().find(Class);
	if (it == GetImportDialogs().end())
		return nullptr;

	return it->second->dialogFunc(parent);
}
