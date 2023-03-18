#pragma once

#include "Windows/ToolsWindow.h"
#include "Object/Object.h"

class QMenu;
class QLabel;
class QToolBar;
class QComboBox;
class CConsoleWidget;
class COutlinerWidget;
class CPropertiesWidget;
class CAssetBrowserDW;
class CWorldViewportWidget;
class CCameraComponent;

class SDK_API CEditorWindow : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CEditorWindow, "Editor Window", false)

public:
	CEditorWindow();
	virtual ~CEditorWindow();

protected:
	virtual bool Shutdown() override;
	virtual void SetupUi() override;

	void SetupMenuBar();

	bool AttempSave();

	void NewScene();
	void LoadScene();
	bool SaveScene(bool bNewPath = false);

	void closeEvent(QCloseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

	void UserSaveState(QSettings& in) override;
	void UserRestoreState(QSettings& in) override;

	void UpdateTitle();

	void OnLevelChange();

	void OnObjectSelected(const TArray<TObjectPtr<CObject>>& objs);

private:
	CConsoleWidget* consoleWidget;
	COutlinerWidget* outlinerWidget;
	CPropertiesWidget* propertiesWidget;
	CAssetBrowserDW* assetBrowser;
	CWorldViewportWidget* worldViewport;
	QLabel* projectLabel;
	QLabel* fpsLabel;
	QComboBox* editorModeCB;

	QToolBar* toolbar;

	QMenu* menuFile;
	QMenu* menuEdit;
	QMenu* menuView;
	QMenu* menuTools;
	QMenu* menuDebug;

	double fpsAvarage;

};
