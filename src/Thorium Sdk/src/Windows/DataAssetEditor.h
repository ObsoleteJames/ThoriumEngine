#pragma once

#include "ToolsWindow.h"
#include "Object/Object.h"

class CDataAsset;
class QVBoxLayout;
class IBasePropertyEditor;
class CCollapsableWidget;

class CDataAssetEditor : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CDataAssetEditor, "Data Asset Editor", true)

public:
	bool Shutdown() final;
	void SetupUi() final;

protected:
	void closeEvent(QCloseEvent* event) override;

private:
	bool ShutdownSave();

	void SetAsset(CDataAsset* asset, bool bNew = false);
	void Save(bool bNew = false);

	void New();
	void Open();

	void UpdateUI();
	void UpdateTitle();

	CCollapsableWidget* GetHeader(const FString&);

private:
	bool bRequiresSave = false;
	bool bReadOnly = false;

	TObjectPtr<CDataAsset> asset;

	QVBoxLayout* l2;

	TArray<CCollapsableWidget*> curWidgets;

};
