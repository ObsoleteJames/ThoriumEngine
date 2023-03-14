#pragma once

#include "Engine.h"
#include "ToolsCore.h"
#include "Rendering/RenderCommands.h"
#include "Registry/RegistryBase.h"
#include <Util/Event.h>

#include <QIcon>

class CMaterial;
class CCameraComponent;
class CEditorEngine;
class CModelAsset;
class CEditorMode;
class CTransformGizmoEntity;
class CObject;

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

	void LoadEditorConfig();
	void SaveEditorConfig();

	void OnLevelChange();

	const TArray<FEditorTheme>& GetThemes() const { return themes; }

	void GenerateGrid(float gridSize, float quadSize, FMesh*& outMesh);

	void SetTheme(const FString& name);

	QIcon GetIcon(const WString& name);
	QIcon GetResourceIcon(const WString& extension);

	const TArray<TObjectPtr<CObject>>& GetSelectedObjects() const { return selectedObjects; }
	void SetSelectedObjects(const TArray<TObjectPtr<CObject>>& objs) { selectedObjects = objs; OnObjectSelected.Fire(selectedObjects); }

	void SetSelectedObject(CObject* obj) { selectedObjects.Clear(); selectedObjects.Add(obj); OnObjectSelected.Fire(selectedObjects); }
	void AddSelectedObject(CObject* obj) { selectedObjects.Add(obj); OnObjectSelected.Fire(selectedObjects); }

	void SetEditorMode(const FString& modeName);
	inline CEditorMode* GetEditorMode() const { return editorMode; }

private:
	void __OnObjectSelected(const TArray<TObjectPtr<CObject>>& obj);

public:
	FEditorConfig config;
	TArray<FEditorTheme> themes;
	TMap<std::wstring, QIcon> themeIcons;

	CEditorMode* editorMode = nullptr;

	TEvent<const TArray<TObjectPtr<CObject>>&> OnObjectSelected;

	// Wether the scene needs to be saved.
	bool bIsPlaying = false;

	FVector gizmoPos;

	TArray<TObjectPtr<CObject>> selectedObjects;

	TObjectPtr<CMaterial> gridMat;
	TObjectPtr<CTransformGizmoEntity> transformGizmo;

	CCameraComponent* editorCamera;
	FMesh* gridMesh;
};

inline CEditorEngine* gEditorEngine() { return static_cast<CEditorEngine*>(gEngine); }
