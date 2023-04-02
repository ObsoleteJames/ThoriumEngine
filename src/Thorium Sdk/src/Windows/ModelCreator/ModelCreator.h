#pragma once

#include "Windows/ToolsWindow.h"
#include "Resources/ModelAsset.h"

class IBasePropertyEditor;
class CWorldViewportWidget;
class CCollapsableWidget;
class CCameraProxy;
class CModelComponent;
class CWorld;
class MCSkeletonWidget;
class CMaterial;
class QTreeWidget;
struct FMesh;
struct aiScene;
struct aiMesh;

class CMaterialWidget;
class CMeshWidget;

struct FImportedMesh
{
	WString file;
	FString name;
	FVector position;
	FVector rotation;
	float scale = 1.f;
	TMap<std::string, bool> objects; // Name, bIncluded
};

struct FModelAssetData
{
public:
	TArray<FImportedMesh> importedMeshes;

public:
	void Clear() {
		importedMeshes.Clear();
	}

};

class CModelCreator : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CModelCreator, "Model Creator", true)

	friend class CMeshWidget;

public:
	CModelCreator() = default;

	bool Shutdown() override;
	void SetupUi() override;

	void Init();

	void UserSaveState(QSettings& settings) override;
	void UserRestoreState(QSettings& settings) override;

	void SetModel(CModelAsset* model, bool bNew = false);

protected:
	void paintEvent(QPaintEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

private:
	bool ShutdownSave();

	void UpdateUI();
	void Compile();

	void CompileMesh(const aiScene* scene, const aiMesh* importMesh, const FVector& offset, const FQuaternion& rotation, const FVector& scale);

	void UpdateMaterials();
	void UpdateSkeleton();

	void AddMesh();
	void RemoveMesh(SizeType id);

	void UpdateMesh(SizeType id);

	void NewModel();
	void LoadModel();
	void SaveModel(bool bNewPath = false);

	inline CMeshWidget* GetMeshWidget(SizeType index) { if (index < meshWidgets.Size()) { return meshWidgets[index]; } return nullptr; }

	inline void MarkDirty() { bRequiresSave = true; UpdateTitle(); }

	void UpdateTitle();

	// Clears the UI mesh list
	void ClearMeshList();

private:
	bool bRequiresSave = false;
	bool bReadOnly = false;
	bool bAutoCompile = true;

	TObjectPtr<CWorld> world;
	TObjectPtr<CModelAsset> model;
	//TObjectPtr<CCameraComponent> camera;
	CCameraProxy* camera;

	FModelAssetData data;

	FMesh* gridMesh;
	TObjectPtr<CMaterial> gridMat;

private:
	QDockWidget* detailsWidget;
	QDockWidget* skeletonWidget;
	//MCSkeletonWidget* skeletonWidget;

	QTreeWidget* skeletonTree;

	QWidget* meshesWidget;
	CCollapsableWidget* meshList;
	TArray<CMeshWidget*> meshWidgets;

	QWidget* materialsWidget;
	CCollapsableWidget* materialsList;
	TArray<IBasePropertyEditor*> materialWidgets;

	TObjectPtr<CModelComponent> modelComp;

	CWorldViewportWidget* viewport;

};
