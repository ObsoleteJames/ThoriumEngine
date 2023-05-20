
#include <string>
#include "CreateCppClassDialog.h"
#include "Object/Object.h"
#include "Object/Class.h"
#include "Module.h"
#include "Console.h"
#include "Engine.h"
#include "CodeGenerator.h"

#include <QBoxLayout>
#include <QTreeWidget>
#include <QListView>
#include <QSpacerItem>
#include <QPushButton>
#include <QLineEdit>
#include "Widgets/TreeDataItem.h"

CCreateCppClassDialog::CCreateCppClassDialog(QWidget* parent /*= nullptr*/) : CFramelessDialog(parent)
{
	QFrame* frame = new QFrame(this);
	QVBoxLayout* layout = new QVBoxLayout(frame);
	//{
	//	QVBoxLayout* l = new QVBoxLayout(this);
	//	l->setContentsMargins(0, 0, 0, 0);
	//	l->addWidget(frame);
	//}

	setCentralWidget(frame);
	setTitle("Create C++ Class");

	filterClass = nullptr;
	selectedClass = nullptr;

	classTree = new QTreeWidget(this);
	classTree->setColumnCount(1);
	classTree->setHeaderLabel("Class");
	layout->addWidget(classTree);

	QHBoxLayout* l1 = new QHBoxLayout();
	l1->addWidget(new QLabel("Name: ", this));
	l1->setContentsMargins(0, 0, 0, 0);

	nameEdit = new QLineEdit(this);
	nameEdit->setPlaceholderText("Class Name...");
	l1->addWidget(nameEdit);
	layout->addLayout(l1);

	QHBoxLayout* l2 = new QHBoxLayout();
	l2->addWidget(new QLabel("Location: ", this));
	l2->setContentsMargins(0, 0, 0, 0);

	pathEdit = new QLineEdit(this);
	l2->addWidget(pathEdit);

	layout->addLayout(l2);

	QWidget* buttonsArea = new QWidget(this);
	QHBoxLayout* buttonsLayout = new QHBoxLayout(this);
	buttonsArea->setLayout(buttonsLayout);

	buttonsLayout->addItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));

	selectButton = new QPushButton("Select", buttonsArea);
	cancelButton = new QPushButton("Cancel", buttonsArea);

	selectButton->setProperty("type", QVariant("primary"));
	selectButton->setEnabled(false);

	buttonsLayout->addWidget(selectButton);
	buttonsLayout->addWidget(cancelButton);

	layout->addWidget(buttonsArea);

	resize(680, 400);

	connect(classTree, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem* item, int) { 
		if (!nameEdit->text().isEmpty())
			selectButton->setEnabled(true); 
		selectedClass = ((TTreeDataItem<FClass*>*)item)->GetData(); 
	});
	connect(nameEdit, &QLineEdit::textChanged, this, [=](const QString& t) {
		if (!t.isEmpty() && !selectedClass)
			selectButton->setEnabled(true);
	});

	connect(selectButton, &QPushButton::clicked, this, [=](bool) { if (!selectedClass) return; Finish(); done(true); deleteLater(); });
	connect(cancelButton, &QPushButton::clicked, this, [=](bool) { done(false); deleteLater(); });
}

CCreateCppClassDialog::~CCreateCppClassDialog()
{
}

int CCreateCppClassDialog::exec()
{
	if (!filterClass)
		filterClass = CObject::StaticClass();

	// Generate tree
	{
		TTreeDataItem<FClass*>* item = new TTreeDataItem<FClass*>(filterClass, (QTreeWidget*)nullptr);
		item->setText(0, filterClass->GetName().c_str());
		classTree->insertTopLevelItem(0, item);

		TArray<FClass*> children;
		CModuleManager::FindChildClasses(filterClass, children);

		for (FClass* child : children)
			AddClassItem(child, item);
	}

	classTree->expandAll();

	return QDialog::exec();
}

void CCreateCppClassDialog::AddClassItem(FClass* c, void* p)
{
	TTreeDataItem<FClass*>* parent = (TTreeDataItem<FClass*>*)p;
	TTreeDataItem<FClass*>* item = nullptr;

	if (!c->HasFlag(CTAG_HIDDEN))
	{
		item = new TTreeDataItem<FClass*>(c, parent);
		item->setText(0, c->GetName().c_str());
	}
	else
		item = parent;

	TArray<FClass*> children;
	CModuleManager::FindChildClasses(c, children);

	for (FClass* child : children)
		AddClassItem(child, item);
}

void CCreateCppClassDialog::Finish()
{
	if (nameEdit->text().isEmpty())
		return;

	FString path = nameEdit->text().toStdString();
	if (!pathEdit->text().isEmpty())
		path = FString(pathEdit->text().toStdString()) + path;

	CCodeGenerator::GenerateCppFile(path, selectedClass->GetInternalName());
}
