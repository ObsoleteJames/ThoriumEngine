#pragma once

#include "AdvancedDockWidgets/DockWidget.h"
#include "Widgets/AssetBrowser.h"

class CAssetBrowserDW : public ads::CDockWidget
{
	Q_OBJECT

public:
	CAssetBrowserDW(QWidget* parent);

public:
	CAssetBrowserWidget* assetBrowser;

};
