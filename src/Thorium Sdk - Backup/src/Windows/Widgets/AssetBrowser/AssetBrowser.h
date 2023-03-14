#pragma once

#include <QDockWidget>

class CAssetBrowserWidget : public QDockWidget
{
	Q_OBJECT

public:
	CAssetBrowserWidget(QWidget* parent = nullptr);
	virtual ~CAssetBrowserWidget();

private:
	void OnAssetUpdate();

private:


};
