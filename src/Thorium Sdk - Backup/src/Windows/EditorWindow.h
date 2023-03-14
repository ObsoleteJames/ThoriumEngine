#pragma once

#include "ToolsWindow.h"

class QMenu;
class QLabel;
class CConsoleWidget;
class CAssetBrowserWidget;
class CWorldViewportWidget;
class CCameraComponent;

class CEditorWindow : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CEditorWindow, "Editor Window", false)

public:
	CEditorWindow();
	virtual ~CEditorWindow();

protected:
	virtual bool Shutdown() override;
	virtual void SetupUi() override;

	void closeEvent(QCloseEvent *event) override;
	void paintEvent(QPaintEvent *event) override;

private:
	CConsoleWidget* consoleWidget;
	CAssetBrowserWidget* assetBrowser;
	CWorldViewportWidget* worldViewport;
	QLabel* projectLabel;
	QLabel* fpsLabel;

	QMenu* menuFile;
	QMenu* menuEdit;
	QMenu* menuView;
	QMenu* menuTools;
	QMenu* menuDebug;

	double fpsAvarage;

};
