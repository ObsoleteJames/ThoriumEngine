
#include "ModuleDebugger.h"
#include "Module.h"

#include <QBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QTreeWidget>
#include <QSplitter>
#include <QLineEdit>

SDK_REGISTER_WINDOW(CModuleDebugger, "Module Debugger", "Debug/Module Debugger", NULL);

enum ETreeModuleItemType
{
	MIT_MODULE,
	MIT_CLASS,
	MIT_STRUCT,
	MIT_ASSET,
	MIT_ENUM,
};

class CTreeModuleItem : public QTreeWidgetItem
{
	//Q_OBJECT

public:
	inline CTreeModuleItem(QTreeWidget* parent, int type) : QTreeWidgetItem(parent, type) {}
	inline CTreeModuleItem(QTreeWidgetItem* parent, int type) : QTreeWidgetItem(parent, type) {}

public:
	void* objPtr = nullptr;

};

bool CModuleDebugger::Shutdown()
{
	return true;
}

void CModuleDebugger::SetupUi()
{
	CToolsWindow::SetupUi();

	setGeometry(QRect(0, 0, 640, 430));
	setWindowTitle("Module Debugger");

	Content = new QWidget(this);
	auto layout = new QVBoxLayout();
	Content->setLayout(layout);

	//Content->setStyleSheet("QWidget { background-color: #38393C; }");

	Properties = new QWidget(Content);
	Properties->setLayout(new QVBoxLayout());

	PropertiesScroll = new QScrollArea(Content);
	PropertiesScroll->setWidget(Properties);

	treeView = new QTreeWidget(Content);
	treeView->setColumnCount(2);
	treeView->setColumnWidth(0, 200);
	//treeView->setAlternatingRowColors(true);
	treeView->setHeaderLabels({ "Name", "Type" });

	connect(treeView, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem* item, int) { UpdatePropertyList((CTreeModuleItem*)item); });

	QSplitter* splitter = new QSplitter(Content);
	splitter->addWidget(treeView);
	splitter->addWidget(PropertiesScroll);
	splitter->setStretchFactor(0, 0);
	splitter->setStretchFactor(1, 1);

	layout->addWidget(splitter);
	setCentralWidget(Content);

	GenerateTree();
}

void CModuleDebugger::UpdatePropertyList(CTreeModuleItem* item)
{
	QLayout* layout = Properties->layout();
	//for (auto* w : layout->findChildren<QWidget*>())
	//{
	//	layout->removeWidget(w);
	//	w->deleteLater();
	//}
	
	switch (item->type())
	{
	case MIT_CLASS:
	{
		FClass* clas = (FClass*)item->objPtr;

		// Name
		{
			QWidget* pWidget = new QWidget(Properties);
			QHBoxLayout* pLayout = new QHBoxLayout();
			pWidget->setLayout(pLayout);

			QLabel* pLabel = new QLabel("Name: ", pWidget);
			pLabel->setMinimumSize({ 100, 0 });

			QLineEdit* pEdit = new QLineEdit(pWidget);
			pEdit->setReadOnly(true);
			pEdit->setText(clas->name.c_str());

			pLayout->addWidget(pLabel);
			pLayout->addWidget(pEdit);

			layout->addWidget(pWidget);
		}
		// Internal Name
		{
			QWidget* pWidget = new QWidget(Properties);
			QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
			pWidget->setLayout(pLayout);

			QLabel* pLabel = new QLabel("Internal Name: ", pWidget);
			pLabel->setMinimumSize({ 100, 0 });

			QLineEdit* pEdit = new QLineEdit(pWidget);
			pEdit->setReadOnly(true);
			pEdit->setText(clas->cppName.c_str());

			pLayout->addWidget(pLabel);
			pLayout->addWidget(pEdit);

			layout->addWidget(pWidget);
		}
		// Description
		{
			QWidget* pWidget = new QWidget(Properties);
			QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
			pWidget->setLayout(pLayout);

			QLabel* pLabel = new QLabel("Description: ", pWidget);
			pLabel->setMinimumSize({ 100, 0 });

			QLineEdit* pEdit = new QLineEdit(pWidget);
			pEdit->setReadOnly(true);
			pEdit->setText(clas->description.c_str());

			pLayout->addWidget(pLabel);
			pLayout->addWidget(pEdit);

			layout->addWidget(pWidget);
		}
		// Size
		{
			QWidget* pWidget = new QWidget(Properties);
			QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
			pWidget->setLayout(pLayout);

			QLabel* pLabel = new QLabel("Size: ", pWidget);
			pLabel->setMinimumSize({ 100, 0 });

			QLineEdit* pEdit = new QLineEdit(pWidget);
			pEdit->setReadOnly(true);
			pEdit->setText(QString::number(clas->Size()));

			pLayout->addWidget(pLabel);
			pLayout->addWidget(pEdit);

			layout->addWidget(pWidget);
		}
		// Base Class
		if (clas->GetBaseClass())
		{
			QWidget* pWidget = new QWidget(Properties);
			QHBoxLayout* pLayout = new QHBoxLayout(pWidget);
			pWidget->setLayout(pLayout);

			QLabel* pLabel = new QLabel("Base Class: ", pWidget);
			pLabel->setMinimumSize({ 100, 0 });

			QLineEdit* pEdit = new QLineEdit(pWidget);
			pEdit->setReadOnly(true);
			pEdit->setText(clas->GetBaseClass()->name.c_str());

			pLayout->addWidget(pLabel);
			pLayout->addWidget(pEdit);

			layout->addWidget(pWidget);
		}

		{
			QLabel* label = new QLabel("Properties:", Properties);
			label->setMinimumSize({ 0, 30 });
			layout->addWidget(label);

			QFrame* line = new QFrame(Properties);
			line->setFrameShadow(QFrame::Sunken);
			line->setLineWidth(2);
			line->setFrameShape(QFrame::HLine);

			layout->addWidget(line);
		}

		//for (const FProperty* p = clas->GetPropertyList(); p != nullptr; p = p->next)
		//{
		//	
		//}
	}
		break;
	}
}

void CModuleDebugger::GenerateTree()
{
	TArray<CModule*> ModulesList = CModuleManager::GetModules();

	// Delete removed modules
	for (SizeType i = 0; i < treeView->invisibleRootItem()->childCount(); i++)
	{
		CTreeModuleItem* it = (CTreeModuleItem*)treeView->invisibleRootItem()->child(i);
		bool bExists = false;
		for (auto _module : ModulesList)
		{
			if (_module == it->objPtr)
			{
				bExists = true;
				break;
			}
		}

		if (!bExists)
			treeView->invisibleRootItem()->removeChild(it);
	}

	for (auto _module : ModulesList)
	{
		CTreeModuleItem* moduleItem = nullptr;
		for (SizeType i = 0; i < treeView->invisibleRootItem()->childCount(); i++)
		{
			CTreeModuleItem* it = (CTreeModuleItem*)treeView->invisibleRootItem()->child(i);
			if (_module == it->objPtr)
			{
				moduleItem = it;
				break;
			}
		}

		if (!moduleItem)
		{
			moduleItem = new CTreeModuleItem(treeView, MIT_MODULE);
			moduleItem->objPtr = _module;
		}

		moduleItem->setText(0, _module->Name().c_str());
		moduleItem->setText(1, "CModule");

		for (auto _class : _module->Classes)
		{
			auto* item = new CTreeModuleItem(moduleItem, MIT_CLASS);
			item->objPtr = _class;

			item->setText(0, _class->GetName().c_str());
			item->setText(1, "FClass");
		}
		for (auto _class : _module->Structures)
		{
			auto* item = new CTreeModuleItem(moduleItem, MIT_STRUCT);
			item->objPtr = _class;

			item->setText(0, _class->GetName().c_str());
			item->setText(1, "FStruct");
		}
		for (auto _class : _module->Enums)
		{
			auto* item = new CTreeModuleItem(moduleItem, MIT_ENUM);
			item->objPtr = _class;

			item->setText(0, _class->GetName().c_str());
			item->setText(1, "FEnum");
		}
		for (auto _class : _module->Assets)
		{
			auto* item = new CTreeModuleItem(moduleItem, MIT_ASSET);
			item->objPtr = _class;

			item->setText(0, _class->GetName().c_str());
			item->setText(1, "FAssetClass");
		}
	}
}
