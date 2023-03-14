#pragma once

#include <QWidget>
#include <QTreeWidget>

class CAssetTreeWidget : public QTreeWidget
{
public:
	CAssetTreeWidget(QWidget* parent = nullptr);
	virtual ~CAssetTreeWidget();

	void UpdateList();

private:


};
