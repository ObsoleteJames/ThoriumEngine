
#include "AssetBrowserDock.h"

#include "Widgets/AssetBrowser.h"

CAssetBrowserDW::CAssetBrowserDW(QWidget* parent) : QDockWidget(parent)
{
	setWindowTitle("Asset Browser");
	setObjectName("assetbrowser_dockwidget");

	assetBrowser = new CAssetBrowserWidget(this);
	setWidget(assetBrowser);
}
