#pragma once

#include <QDockWidget>
#include <QMenu>
#include <Util/String.h>
#include "ToolsCore.h"

struct FDirectory;
struct FFile;
struct FMod;
class FAssetClass;
class QLineEdit;
class QListWidget;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class QSlider;

class CAssetFilterMenu : public QMenu
{
	Q_OBJECT

public:
	CAssetFilterMenu(TArray<WString>* filterList, QWidget* parent = nullptr);
	~CAssetFilterMenu();

	void SetFilter(WString ext, bool enabled);

Q_SIGNALS:
	void OnFilterUpdate();

private:
	TArray<WString>* filterList;
	TArray<QAction*> actions;

};

struct SDK_API FAssetCreateMenu
{
	QString name;
	void(*func)(const WString& path);
};

class SDK_API CAssetBrowserWidget : public QWidget
{
	Q_OBJECT

public:
	enum EViewMode
	{
		VIEW_GRID,
		VIEW_LIST
	};

public:
	CAssetBrowserWidget(QWidget* parent = nullptr);
	virtual ~CAssetBrowserWidget();

	/*
	 *	Sets the current directory.
	 * 
	 *	Format: "Mod:\Dir\Dir2\etc..."
	 */
	void SetDirectory(const WString& dir, bool fromHistory = false);
	inline WString GetDirectory() const { return curDir; }

	void SetGridSize(int size);
	inline int GridSize() const { return dirViewSize; }

	void AllowFileCreation() { bCreateFiles = true; }
	void DisableFileCreation() { bCreateFiles = false; }

	void LockAssetFilter();
	inline void AddAssetFilter(WString fileExt) { activeFilters.Add(fileExt); UpdateView(); }

	inline EViewMode ViewMode() const { return bDirViewGrid ? VIEW_GRID : VIEW_LIST; }
	inline void SetViewMode(EViewMode vm) { bDirViewGrid = vm == VIEW_GRID; }

	inline FFile* SelectedFile() const { return selectedFile; }
	inline const TArray<FFile*>& SelectedFiles() const { return selectedFiles; }

	inline QSplitter* GetSplitter() const { return splitter; }

	static void RegisterAssetCreateMenu(const FAssetCreateMenu& cm) { assetMenus.Add(cm); }
	static void RegisterAssetCreateMenu(const QString& name, void(*func)(const WString& path)) { assetMenus.Add({ name, func }); }

private:
	void OnAssetUpdate();

	void UpdateView();
	void UpdateViewSettings();

	FDirectory* GetFDirectory(const WString& path);

	void AddDirToTree(FMod* mod, FDirectory* dir, QTreeWidgetItem* parent);
	void CreateContextMenu(QPoint point);

	void ImportAsset();

	static TArray<FAssetCreateMenu> assetMenus;

Q_SIGNALS:
	void fileDoubleClicked();
	void fileClicked();

private:
	int dirViewSize = 3;
	bool bCreateFiles = true;
	bool bDirViewGrid = true;
	bool bFiltersLocked = false;

	TArray<WString> activeFilters;

	SizeType historyIndex = 0;
	TArray<WString> dirHistory;
	WString curDir;

	CAssetFilterMenu* filterMenu;
	QTreeWidget* fileTree;
	QListWidget* dirView;

	QLineEdit* curFolderEdit;

	FFile* selectedFile;
	TArray<FFile*> selectedFiles;

	QSlider* gridSizeSlider;
	QPushButton* btnViewMode;
	QPushButton* btnRootFolder;
	QPushButton* btnGoForward;
	QPushButton* btnGoBack;
	QPushButton* btnFilters;
	QPushButton* btnCreateAsset;
	QSplitter* splitter;

};
