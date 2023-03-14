#pragma once

#include <QDockWidget>
#include "Widgets/AssetBrowser.h"

class CAssetBrowserDW : public QDockWidget
{
	Q_OBJECT

public:
	CAssetBrowserDW(QWidget* parent);

public:
	CAssetBrowserWidget* assetBrowser;

};
