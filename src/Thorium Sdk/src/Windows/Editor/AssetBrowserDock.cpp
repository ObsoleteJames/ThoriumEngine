
#include "AssetBrowserDock.h"

#include "Widgets/AssetBrowser.h"

CAssetBrowserDW::CAssetBrowserDW(QWidget* parent) : ads::CDockWidget("Asset Browser", parent)
{
	setObjectName("assetbrowser_dockwidget");

	assetBrowser = new CAssetBrowserWidget(this);
	setWidget(assetBrowser);
}
