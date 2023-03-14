#pragma once

#include <QDialog>
#include "FramelessDialog.h"
#include <Util/Core.h>

struct FFile;
struct FAssetClass;
class CAssetBrowserWidget;
class QLineEdit;

class CSaveFileDialog : public CFramelessDialog
{
	Q_OBJECT

public:
	CSaveFileDialog(QWidget* parent = nullptr);

	inline WString Path() const { return path; }

private:
	void Save();

private:
	WString path;
	CAssetBrowserWidget* assetBrowser;
	QLineEdit* nameEdit;

};

class COpenFileDialog : public CFramelessDialog
{
	Q_OBJECT

public:
	COpenFileDialog(const WString& filter, QWidget* parent = nullptr);
	COpenFileDialog(FAssetClass* filterType, QWidget* parent = nullptr);

	inline FFile* File() const { return file; }
	
private:
	FFile* file;
	CAssetBrowserWidget* assetBrowser;

};
