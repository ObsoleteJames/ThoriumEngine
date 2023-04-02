#pragma once

#include "Engine.h"
#include "ToolsCore.h"
#include "Rendering/RenderCommands.h"
#include "Registry/RegistryBase.h"
#include "HistoryBuffer.h"
#include "Object/Delegate.h"

#include <QIcon>

class CMaterial;
class CCameraProxy;
class CEditorEngine;
class CModelAsset;
class CEditorMode;
class CTransformGizmoEntity;
class CObject;
class CEditorWindow;

struct FMesh;

struct SDK_API FEditorConfig
{
	FString theme;
};

struct SDK_API FEditorTheme
{
	FString name;
	FString displayName;
};

CREATE_OBJECT_REGISTRY_DLL(CEditorModeRegistry, CEditorMode*, SDK_API);

class SDK_API CEditorEngine : public CEngine
{
public:
	void Init() override;

	// Instead of running a loop in here, this gets called by the Qt event loop.
	int Run() override;

	virtual void OnExit();

public:
	//inline void SetRenderScene(CRenderScene* scene) { worldRenderScene = scene; }
	inline void SetDeltaTime(double dt) { deltaTime = dt; }
	inline void UpdateRenderTime(double t) { renderTime = t; }

	void LoadEditorConfig();
	void SaveEditorConfig();

	void OnLevelChange();

	const TArray<FEditorTheme>& GetThemes() const { return themes; }

	void GenerateGrid(float gridSize, float quadSize, FMesh*& outMesh);

	void SetTheme(const FString& name);

	QIcon GetIcon(const WString& name);
	QIcon GetResourceIcon(const WString& extension);

	const TArray<TObjectPtr<CObject>>& GetSelectedObjects() const { return selectedObjects; }
	void SetSelectedObjects(const TArray<TObjectPtr<CObject>>& objs) { selectedObjects = objs; OnObjectSelected.Invoke(selectedObjects); }

	void SetSelectedObject(CObject* obj);
	void AddSelectedObject(CObject* obj) { selectedObjects.Add(obj); OnObjectSelected.Invoke(selectedObjects); }

	void SetEditorMode(const FString& modeName);
	inline CEditorMode* GetEditorMode() const { return editorMode; }

	void RegisterProject(const FProject& proj);

private:
	void __OnObjectSelected(const TArray<TObjectPtr<CObject>>& obj);

	CEditorMode* editorMode = nullptr;

public:
	CEditorWindow* editorWindow;

	FEditorConfig config;
	TArray<FEditorTheme> themes;
	TMap<std::wstring, QIcon> themeIcons;

	TArray<FProject> availableProjects;

	CHistoryBuffer historyBuffer;
	TDelegate<const TArray<TObjectPtr<CObject>>&> OnObjectSelected;

	bool bIsPlaying = false;

	// Wether changes have been made to the loaded scene.
	bool bSceneDirty = false;
	SizeType savedAtHC = -1;

	FVector gizmoPos;

	TArray<TObjectPtr<CObject>> selectedObjects;

	TObjectPtr<CMaterial> gridMat;
	TObjectPtr<CTransformGizmoEntity> transformGizmo;

	CCameraProxy* editorCamera;
	FMesh* gridMesh;
};

inline CEditorEngine* gEditorEngine() { return static_cast<CEditorEngine*>(gEngine); }
