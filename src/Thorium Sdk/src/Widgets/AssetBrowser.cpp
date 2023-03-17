
#include "AssetBrowser.h"
#include "Resources/Asset.h"
#include "Engine.h"
#include "Widgets/TreeDataItem.h"
#include "Console.h"
#include "Object/Class.h"
#include "ImportDialogs/BaseImportDialog.h"
#include "ToolsCore.h"
#include "FramelessDialog.h"
#include "EditorEngine.h"

#include <QSplitter>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QSlider>
#include <QDropEvent>
#include <QProcess>
#include <QFileDialog>

#define ASSET_MAX_GRID_SIZE 5

TArray<FAssetCreateMenu> CAssetBrowserWidget::assetMenus;

class CAssetList : public QListWidget
{
public:
	CAssetList(QWidget* parent = nullptr) : QListWidget(parent) {}

protected:
	void dropEvent(QDropEvent* event) { }

};

CAssetFilterMenu::CAssetFilterMenu(TArray<WString>* l, QWidget* parent) : QMenu(parent)
{
	filterList = l;
	
	TArray<FAssetClass*> classes;
	CModuleManager::GetAssetTypes(classes);

	QAction* clearFilter = new QAction("Clear Filters", this);
	connect(clearFilter, &QAction::triggered, this, [=]() {
		for (auto* a : actions)
			a->setChecked(false);
		filterList->Clear();
		emit(OnFilterUpdate());
	});
	addAction(clearFilter);
	addSeparator();

	for (auto* c : classes)
	{
		if (c->Flags() & CTAG_ABSTRACT || c->Flags() & CTAG_HIDDEN)
			continue;
		QAction* action = new QAction(gEditorEngine()->GetResourceIcon(ToWString(c->GetExtension())), c->GetName().c_str());
		action->setCheckable(true);
		connect(action, &QAction::triggered, this, [=](bool b) { this->SetFilter(ToWString(c->GetExtension()), b); });
		addAction(action);
		actions.Add(action);
	}
}

CAssetFilterMenu::~CAssetFilterMenu()
{
}

void CAssetFilterMenu::SetFilter(WString ext, bool enabled)
{
	if (enabled)
	{
		if (filterList->Find(ext) == filterList->end())
			filterList->Add(ext);
	}
	else
	{
		auto it = filterList->Find(ext);
		if (it != filterList->end())
			filterList->Erase(it);
	}

	emit(OnFilterUpdate());
}

CAssetBrowserWidget::CAssetBrowserWidget(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
	setObjectName("assetbrowser_widget");

	QVBoxLayout* layout = new QVBoxLayout();
	setLayout(layout);

	QWidget* dirViewWidget = new QWidget(this);
	QVBoxLayout* dirViewLayout = new QVBoxLayout(dirViewWidget);
	QHBoxLayout* topLayout = new QHBoxLayout();

	QFrame* lineFrame = new QFrame(this);
	lineFrame->setLayout(topLayout);

	filterMenu = new CAssetFilterMenu(&activeFilters, this);

	btnRootFolder = new QPushButton(QIcon(":/icons/arrow-return.svg"), "", this);
	btnGoForward = new QPushButton(QIcon(":/icons/arrow-right.svg"), "", this);
	btnGoBack = new QPushButton(QIcon(":/icons/arrow-left.svg"), "", this);
	btnFilters = new QPushButton(QIcon(":/icons/filter-dropdown.svg"), "Filters", this);
	btnViewMode = new QPushButton(QIcon(":/icons/dropdown.svg"), "", this);
	btnCreateAsset = new QPushButton("+", this);
	curFolderEdit = new QLineEdit(this);

	gridSizeSlider = new QSlider(Qt::Horizontal, this);
	
	fileTree = new QTreeWidget(this);
	dirView = new CAssetList(this);
	dirView->setObjectName("CAssetBrowser::dirView");
	dirView->setContextMenuPolicy(Qt::CustomContextMenu);
	dirView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	connect(dirView, &QListWidget::customContextMenuRequested, this, &CAssetBrowserWidget::CreateContextMenu);

	dirView->setDragDropMode(QListWidget::NoDragDrop);
	connect(dirView, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem* item) {
		if (item->type() == EItemTypes_Folder)
		{
			WString newDir = curDir;
			if (*newDir.last() == L'\\')
				newDir.Erase(newDir.last());
			SetDirectory(newDir + L"\\" + (const wchar_t*)item->text().data());
		}
		else if (item->type() == EItemTypes_AssetFile)
		{
			selectedFile = (FFile*)item->data(257).toULongLong();
			emit(fileDoubleClicked());
		}
	});
	connect(dirView, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
		selectedFile = nullptr;
		if (item->type() == EItemTypes_AssetFile)
		{
			selectedFile = (FFile*)item->data(257).toULongLong();
		}
		emit(fileClicked());
	});
	connect(dirView, &QListWidget::itemSelectionChanged, this, [=]() {
		selectedFiles.Clear();
		auto items = dirView->selectedItems();
		for (auto* item : items)
		{
			if (item->type() == EItemTypes_AssetFile)
			{
				selectedFiles.Add((FFile*)item->data(257).toULongLong());
			}
		}
	});
	connect(fileTree, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem* item, int) {
		if (item->type() == EItemTypes_Folder)
			SetDirectory((const wchar_t*)item->text(1).data());
		else if (item->type() == EItemTypes_ModFolder)
			SetDirectory((const wchar_t*)(item->text(0) + ":\\").data());
	});

	lineFrame->setProperty("type", QVariant(1));

	btnRootFolder->setProperty("type", QVariant("clear"));
	btnRootFolder->setMaximumSize(QSize(24, 24));
	btnRootFolder->setToolTip("Root Folder");
	btnGoForward->setEnabled(false);
	btnGoForward->setProperty("type", QVariant("clear"));
	btnGoForward->setMaximumSize(QSize(24, 24));
	btnGoForward->setToolTip("Forward");
	btnGoBack->setEnabled(false);
	btnGoBack->setProperty("type", QVariant("clear"));
	btnGoBack->setMaximumSize(QSize(24, 24));
	btnGoBack->setToolTip("Back");
	btnFilters->setProperty("type", QVariant("clear"));
	btnFilters->setMaximumSize(QSize(128, 24));
	btnViewMode->setProperty("type", QVariant("clear"));
	btnViewMode->setMaximumSize(QSize(24, 24));
	btnCreateAsset->setProperty("type", QVariant("clear"));
	btnCreateAsset->setMaximumSize(QSize(128, 24));

	gridSizeSlider->setMinimum(1);
	gridSizeSlider->setMaximum(ASSET_MAX_GRID_SIZE);
	gridSizeSlider->setMaximumSize(QSize(100, 24));
	gridSizeSlider->setValue(dirViewSize);

	connect(filterMenu, &CAssetFilterMenu::OnFilterUpdate, this, [=]() { UpdateView(); });
	connect(btnFilters, &QPushButton::clicked, this, [=]() { filterMenu->exec(btnFilters->mapToGlobal(QPoint(0, -filterMenu->sizeHint().height()))); });
	connect(curFolderEdit, &QLineEdit::editingFinished, this, [=]() { SetDirectory(ToWString(curFolderEdit->text().toStdString())); });
	connect(btnGoBack, &QPushButton::clicked, this, [=]() {
		if (historyIndex < dirHistory.Size())
			historyIndex++;

		if (historyIndex >= dirHistory.Size())
			btnGoBack->setEnabled(false);

		SetDirectory(dirHistory[dirHistory.Size() - historyIndex], true);
	});
	connect(btnGoForward, &QPushButton::clicked, this, [=]() {
		/*if (historyIndex > 0)
			historyIndex--;

		if (historyIndex == 0)
			btnGoForward->setEnabled(false);

		SetDirectory(dirHistory[dirHistory.Size() - historyIndex + 1], true);*/
	});
	connect(btnRootFolder, &QPushButton::clicked, this, [=]() { 
		WString newDir = curDir;
		newDir.Erase(newDir.begin() + newDir.FindLastOf('\\'), newDir.end());
		if (*newDir.last() == L':')
			newDir += L"\\";
		SetDirectory(newDir);
	});
	connect(btnViewMode, &QPushButton::clicked, this, [=]() { bDirViewGrid ^= 1; UpdateViewSettings(); });
	connect(gridSizeSlider, &QSlider::valueChanged, this, [=](int value) { dirViewSize = value; UpdateViewSettings(); });
	connect(btnCreateAsset, &QPushButton::clicked, this, [=]() { 
		QMenu menu;

		for (auto& m : assetMenus)
		{
			menu.addAction(m.name, this, [=]() { m.func(curDir); UpdateView(); });
		}
		menu.exec(btnCreateAsset->mapToGlobal(QPoint(0, -btnCreateAsset->sizeHint().height())));
	});

	topLayout->addWidget(btnRootFolder);
	topLayout->addWidget(btnGoBack);
	topLayout->addWidget(btnGoForward);
	topLayout->addWidget(btnFilters);
	topLayout->addWidget(btnCreateAsset);
	topLayout->addWidget(curFolderEdit);
	topLayout->addWidget(btnViewMode);
	topLayout->addWidget(gridSizeSlider);

	fileTree->setHeaderHidden(true);

	splitter = new QSplitter(this);
	splitter->addWidget(fileTree);
	splitter->setStretchFactor(0, 2);
	splitter->addWidget(dirViewWidget);
	splitter->setStretchFactor(1, 6);

	dirViewLayout->setContentsMargins(0, 0, 0, 0);
	layout->setContentsMargins(0, 0, 0, 0);

	topLayout->setSpacing(2);
	topLayout->setContentsMargins(2, 2, 2, 2);
	dirViewLayout->addWidget(lineFrame);
	dirViewLayout->addWidget(dirView);
	layout->addWidget(splitter);

	curDir = ToWString(gEngine->ActiveGame().name) + L":\\";
	
	UpdateViewSettings();

	OnAssetUpdate();
}

CAssetBrowserWidget::~CAssetBrowserWidget()
{

}

void CAssetBrowserWidget::SetDirectory(const WString& d, bool fromHistory)
{
	if (GetFDirectory(d) == nullptr)
	{
		curFolderEdit->setText(QString((const QChar*)curDir.c_str()));
		return;
	}

	if (historyIndex > 0)
	{
		if (!fromHistory)
		{
			SizeType i = dirHistory.Size() - historyIndex;
			dirHistory.Erase(dirHistory.begin() + i, dirHistory.end());
			historyIndex = 0;
			btnGoForward->setEnabled(false);
		}
		else
			btnGoForward->setEnabled(true);
	}

	if (!fromHistory)
	{
		if (dirHistory.Size() == 16)
			dirHistory.Erase(dirHistory.begin());

		btnGoBack->setEnabled(true);
		dirHistory.Add(curDir);
	}

	curDir = d;
	UpdateView();
}

FDirectory* CAssetBrowserWidget::GetFDirectory(const WString& path)
{
	WString mod = path;
	WString dir = path;

	if (auto it = mod.FindFirstOf(':'); mod.begin() + it != mod.end())
	{
		mod.Erase(mod.begin() + it, mod.end());

		if (it + 2 < dir.Size())
			dir.Erase(dir.begin(), dir.begin() + it + 2);
		else
			dir.Clear();
	}
	else
		return nullptr;

	FMod* m = CFileSystem::FindMod(mod);
	if (m == nullptr)
		return nullptr;

	if (dir.IsEmpty())
		return m->GetRootDir();

	if (FDirectory* d = m->FindDirectory(dir); d != nullptr)
		return d;

	return nullptr;
}

void CAssetBrowserWidget::AddDirToTree(FMod* mod, FDirectory* dir, QTreeWidgetItem* parent)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(parent, EItemTypes_Folder);
	item->setText(0, QString((const QChar*)dir->GetName().c_str()));
	WString path = mod->Name() + L":\\" + dir->GetPath();
	item->setText(1, QString((const QChar*)path.c_str()));
	item->setIcon(0, gEditorEngine()->GetIcon(L"folder.svg"));

	for (auto* d : dir->GetSubDirectories())
		AddDirToTree(mod, d, item);
}

void CAssetBrowserWidget::CreateContextMenu(QPoint point)
{
	auto* item = dirView->itemAt(point);

	QMenu menu(this);

	if (item)
	{
		menu.addAction("Delete", this, [=]() {
			if (item->type() == EItemTypes_AssetFile)
			{
				FFile* f = (FFile*)item->data(257).toULongLong();
				f->Mod()->DeleteFile(f->Path());
			}
			else if (item->type() == EItemTypes_Folder)
			{
				WString mod = curDir;
				if (auto it = mod.FindFirstOf(':'); mod.begin() + it != mod.end())
					mod.Erase(mod.begin() + it, mod.end());
				else
					return;

				FDirectory* d = (FDirectory*)item->data(257).toULongLong();
				CFileSystem::FindMod(mod)->DeleteDirectory(d->GetPath());
			}
			UpdateView();
		});

		if (item->type() == EItemTypes_AssetFile)
		{
			menu.addAction("Show in explorer", this, [=]() {
				QStringList params;
				params += "/select,";
				
				FFile* f = (FFile*)item->data(257).toULongLong();
				params += QString((const QChar*)f->FullPath().c_str());

				QProcess::startDetached("explorer.exe", params);

			});

			if (FFile* file = (FFile*)item->data(257).toULongLong(); file && file->Extension() == L".thcs")
			{
				CShaderSource* shader = CResourceManager::GetResource<CShaderSource>(file->Path());
				menu.addAction("Compile", this, [=]() { shader->Compile(); });
			}
		}
	}
	else
	{
		menu.addAction("New Folder", this, [=]() { 
			CFramelessDialog* dialog = new CFramelessDialog(this);

			QFrame* frame = new QFrame(dialog);
			QVBoxLayout* layout = new QVBoxLayout(frame);

			dialog->setCentralWidget(frame);
			dialog->setTitle("New Folder");

			QLineEdit* nameEdit = new QLineEdit(this);
			nameEdit->setPlaceholderText("Name...");

			layout->addWidget(nameEdit);

			QHBoxLayout* l1 = new QHBoxLayout();

			QPushButton* btnImport = new QPushButton("Create", frame);
			btnImport->setProperty("type", QVariant("primary"));
			btnImport->setEnabled(false);
			QPushButton* btnCancel = new QPushButton("Cancel", frame);

			l1->addWidget(btnImport);
			l1->addWidget(btnCancel);

			layout->addLayout(l1);

			connect(nameEdit, &QLineEdit::textChanged, this, [=](const QString& str) { btnImport->setEnabled(!str.isEmpty()); });
			connect(nameEdit, &QLineEdit::returnPressed, this, [=]() { dialog->done(true); });

			connect(btnImport, &QPushButton::clicked, this, [=]() { dialog->done(true); });
			connect(btnCancel, &QPushButton::clicked, this, [=]() { dialog->done(false); });

			if (dialog->exec() && !nameEdit->text().isEmpty())
			{
				WString mod = curDir;
				WString dir = curDir;

				if (auto it = mod.FindFirstOf(':'); mod.begin() + it != mod.end())
				{
					mod.Erase(mod.begin() + it, mod.end());

					if (it + 2 < dir.Size())
						dir.Erase(dir.begin(), dir.begin() + it + 2);
					else
						dir.Clear();
				}

				CFileSystem::FindMod(mod)->CreateDir(dir + L"\\" + nameEdit->text().toStdWString());
				UpdateView();
			}
		});

		if (bCreateFiles)
		{
			menu.addSeparator();

			for (auto& m : assetMenus)
				menu.addAction(m.name, this, [=]() { m.func(curDir); });

			menu.addSeparator();
			menu.addAction("Import Asset", this, [=]() { ImportAsset(); });
		}
	}

	menu.exec(dirView->mapToGlobal(point));
}

void CAssetBrowserWidget::ImportAsset()
{
	FString filter;
	TArray<FAssetClass*> importableClasses;
	for (CModule* m : CModuleManager::GetModules())
	{
		for (FAssetClass* c : m->Assets)
		{
			if (c->ImportableAs().IsEmpty())
				continue;

			TArray<FString> imports = c->ImportableAs().Split(';');

			filter += c->GetName();
			filter += " (";

			for (auto& i : imports)
				filter += "*" + i + " ";

			filter.Erase(filter.last());
			filter += ");;";
			importableClasses.Add(c);
		}
	}

	filter += "All Files (*.*)";

	QStringList file = QFileDialog::getOpenFileNames(this, "Select File...", QString(), filter.c_str());
	if (file.isEmpty())
		return;

	FString ext = file[0].toStdString();
	ext.Erase(ext.begin(), ext.begin() + ext.FindLastOf('.'));

	FAssetClass* targetClass = nullptr;

	// Figure out what the selected file's type is.
	for (auto* c : importableClasses)
	{
		TArray<FString> exts = c->ImportableAs().Split(';');

		for (auto& x : exts)
		{
			if (x == ext)
			{
				targetClass = c;
				break;
			}
		}

		if (targetClass)
			break;
	}

	if (!targetClass)
		return;

	WString dir = curDir;
	WString mod = curDir;

	if (auto it = mod.FindFirstOf(':'); mod.begin() + it != mod.end())
	{
		mod.Erase(mod.begin() + it, mod.end());

		if (it + 2 < dir.Size())
			dir.Erase(dir.begin(), dir.begin() + it + 2);
		else
			dir.Clear();
	}

	if (IImportDialog* dialog = IImportDialog::GetImportDialog(targetClass, this); dialog != nullptr)
	{
		dialog->Exec(file, dir, mod);
		delete dialog;
	}
	else
	{

		FString fileName = file[0].toStdString();
		fileName.Erase(fileName.begin() + fileName.FindLastOf('.'), fileName.end());
		if (auto it = fileName.FindLastOf("/\\"); it != -1)
			fileName.Erase(fileName.begin(), fileName.begin() + it + 1);

		TObjectPtr<CAsset> asset = CResourceManager::CreateResource(targetClass, dir + L"\\" + ToWString(fileName) + ToWString(targetClass->GetExtension()), mod);
		asset->Import(file[0].toStdWString());
	}

	UpdateView();
}

void CAssetBrowserWidget::SetGridSize(int size)
{
	dirViewSize = FMath::Clamp(size, 1, ASSET_MAX_GRID_SIZE);
	gridSizeSlider->setValue(dirViewSize);
	UpdateViewSettings();
}

void CAssetBrowserWidget::LockAssetFilter()
{
	bFiltersLocked = true; 
	btnFilters->setDisabled(true);
}

void CAssetBrowserWidget::OnAssetUpdate()
{
	fileTree->clear();

	const TArray<FMod*>& mods = CFileSystem::GetMods();
	for (auto* m : mods)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(fileTree, EItemTypes_ModFolder);
		item->setText(0, QString((const QChar*)m->Name().c_str()));
		item->setIcon(0, gEditorEngine()->GetIcon(L"folder-blue.svg"));

		for (auto* d : m->GetRootDir()->GetSubDirectories())
			AddDirToTree(m, d, item);
	}

	UpdateView();
}

void CAssetBrowserWidget::UpdateView()
{
	curFolderEdit->setText(ToFString(curDir).c_str());

	dirView->clear();

	FDirectory* dir = GetFDirectory(curDir);
	if (!dir)
		return;

	for (auto* d : dir->GetSubDirectories())
	{
		QListWidgetItem* item = new QListWidgetItem(QString((const QChar*)d->GetName().c_str()), dirView, EItemTypes_Folder);
		item->setData(257, QVariant((SizeType)d));
		item->setIcon(gEditorEngine()->GetIcon(L"folder.svg"));
	}

	for (auto* f : dir->GetFiles())
	{
		if (activeFilters.Size() > 0)
		{
			auto it = activeFilters.Find(f->Extension());
			if (it == activeFilters.end())
				continue;
		}

		QListWidgetItem* item = new QListWidgetItem(QString((const QChar*)f->Name().c_str()), dirView, EItemTypes_AssetFile);
		item->setData(257, QVariant((SizeType)f));
		item->setIcon(gEditorEngine()->GetResourceIcon(f->Extension()));

		WString tooltip = L"Type: " + f->Extension() + L"\nSize: " + WString::ToString(f->Size());
		item->setToolTip(QString((const QChar*)tooltip.c_str()));
	}
}

void CAssetBrowserWidget::UpdateViewSettings()
{
	if (bDirViewGrid)
	{
		btnViewMode->setIcon(QIcon(":/icons/grid-view.svg"));
		gridSizeSlider->setEnabled(true);

		dirView->setFlow(QListView::LeftToRight);
		dirView->setResizeMode(QListView::Adjust);
		dirView->setGridSize(QSize((24 * dirViewSize) + 20, (30 * dirViewSize) + 20));
		dirView->setIconSize(QSize(24 * dirViewSize, 24 * dirViewSize));
		//dirView->setSpacing(2);
		dirView->setViewMode(QListView::IconMode);
	}
	else
	{
		btnViewMode->setIcon(QIcon(":/icons/dropdown.svg"));
		gridSizeSlider->setEnabled(false);

		dirView->setFlow(QListView::TopToBottom);
		dirView->setResizeMode(QListView::Fixed);
		//dirView->setSpacing(2);
		dirView->setGridSize(QSize(-1, -1));
		dirView->setIconSize(QSize(-1, -1));
		dirView->setViewMode(QListView::ListMode);
	}

}
