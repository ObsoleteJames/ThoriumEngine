
#include "AssetBrowser.h"
#include "AssetTree.h"
#include <QSplitter>
#include <QBoxLayout>
#include <QTreeView>

CAssetBrowserWidget::CAssetBrowserWidget(QWidget* parent /*= nullptr*/) : QDockWidget(parent)
{
	setObjectName("assetBrowser");
	setWindowTitle("Asset Browser");

	QWidget* rootWidget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout();
	rootWidget->setLayout(layout);
	setWidget(rootWidget);

	QSplitter* splitter = new QSplitter(rootWidget);
	CAssetTreeWidget* assetTree = new CAssetTreeWidget(rootWidget);
	splitter->addWidget(assetTree);
	splitter->setStretchFactor(0, 1);


	layout->addWidget(splitter);

}

CAssetBrowserWidget::~CAssetBrowserWidget()
{

}

void CAssetBrowserWidget::OnAssetUpdate()
{

}
