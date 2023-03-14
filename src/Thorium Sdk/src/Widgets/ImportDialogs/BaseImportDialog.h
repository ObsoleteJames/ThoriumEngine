#pragma once

#include <QString>
#include <QWidget>
#include "Widgets/FramelessDialog.h"
#include "Object/Class.h"

class IImportDialog;

struct FImportDialogRegister
{
	typedef IImportDialog* (*FuncInstantiateDialog)(QWidget* parent);

public:
	FImportDialogRegister(FAssetClass* target, std::function<IImportDialog* (QWidget*)> func);

public:
	FAssetClass* targetAsset;
	std::function<IImportDialog*(QWidget*)> dialogFunc;
};

class IImportDialog
{
public:
	virtual void Exec(const QStringList& files, const WString& out, const WString& mod) = 0;

	static void RegisterImportDialog(FImportDialogRegister*);
	static IImportDialog* GetImportDialog(FAssetClass* Class, QWidget* parent = nullptr);
};

#define REGISTER_IMPORT_DIALOG(TargetClass, DialogClass) \
static FImportDialogRegister DialogClass##Register((FAssetClass*)##TargetClass##::StaticClass(), [](QWidget* p) { return new DialogClass(p); })
