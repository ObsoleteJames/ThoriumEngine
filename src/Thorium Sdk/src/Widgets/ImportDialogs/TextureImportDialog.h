#pragma once

#include "BaseImportDialog.h"

struct FTextureImportSettings;

class CTextureImportDialog : public IImportDialog
{
public:
	CTextureImportDialog(QWidget* parent = nullptr);

	virtual void Exec(const QStringList& files, const WString& out, const WString& mod);

private:
	void ImportTexture(const QString& file, const FTextureImportSettings& settings);

private:
	WString path;
	WString mod;
	QWidget* parent;

};
