#pragma once

#include "Engine.h"
#include "EditorCore.h"
#include "Rendering/RenderCommands.h"
#include "Object/Delegate.h"
#include "Window.h"
#include "CameraController.h"

#include "Rendering/Framebuffer.h"

#define ITexture2D_ImGui(tex) (((DirectXTexture2D*)tex)->view)

class CLayer;
class CEntity;

class CPropertyEditor;
class CConsoleWidget;
class CInputOutputWidget;
class CProjectSettingsWidget;
class CAssetBrowserWidget;
class CEditorSettingsWidget;
class CAddonsWindow;
class FClass;

class CObjectDebugger;

class SDK_API CEditorEngine : public CEngine
{
public:
	void Init() override;

	int Run() override;

	void OnExit() override;

	void UpdateEditor();

public:
	void LoadEditorConfig();
	void SaveEditorConfig();

	void GenerateBuildData();

	void RegisterProject(const FProject& proj);

	bool IsEntitySelected(CEntity* ent);
	void AddSelectedEntity(CEntity* ent);
	void RemoveSelectedEntity(CEntity* ent);
	void SetSelectedEntity(CEntity* ent);

	template<typename T>
	T* AddLayer() { T* r = new T(); AddLayer(r); return r; }

	void AddLayer(TObjectPtr<CLayer> layer);
	void RemoveLayer(TObjectPtr<CLayer> layer);

	// Remove layer the next frame
	void PollRemoveLayer(TObjectPtr<CLayer> layer);

	void KeyEventA(EKeyCode key, EInputAction action, EInputMod mod);

	void SaveProjectConfig();

private:
	void InitEditorData();

	void NewScene();
	bool SaveScene();

	void StartPlay();
	void StopPlay();

	void DoMousePick();
	void DoEntRightClick();

	// Handle asset dropping
	void DoMaterialDrop(TObjectPtr<CMaterial> mat, bool bPeek);
	void DoModelAssetDrop(TObjectPtr<CModelAsset> mdl, bool bPeek);

	void OnLevelChange();
	
	void ToggleGameInput();

	void DrawSelectionDebug();
	void FocusOnSelection();

	void OutlinerDrawEntity(CEntity* ent, bool bRoot = true);
	void EntityContextMenu(CEntity* ent, const FVector& clickPos);
	void DoEntityShortcuts();

	void CopyEntity();
	void PasteEntity(const FVector& pos);

	void SceneFileDialogs();

	void DupeEntity();

public:
	IFrameBuffer* sceneFrameBuffer;
	//IDepthBuffer* sceneDepthBuffer;
	int viewportWidth, viewportHeight;
	float viewportX = 0.f, viewportY = 0.f;

	TArray<FProject> availableProjects;

	TArray<CLayer*> removeLayers;
	TArray<TObjectPtr<CLayer>> layers;

	TArray<TObjectPtr<CEntity>> selectedEntities;
	TObjectPtr<CObject> selectedObject;

	CCameraProxy* editorCamera;
	FMesh* gridMesh;
	TObjectPtr<CMaterial> gridMat;

	bool bIsPlaying = false;
	bool bPaused = false;
	bool bStepFrame = false;

	bool bViewOutliner = true;
	bool bViewAssetBrowser = true;
	bool bViewStats = false;

	bool bImGuiDemo = false;

	double imguiRenderTime;
	double editorUpdateTime;

	struct {
		int wndPosX = 100;
		int wndPosY = 100;
		int wndWidth = 1600;
		int wndHeight = 900;
		int wndMode = 1;
	} editorCfg;

	enum ECopyBufferData
	{
		CB_NONE,
		CB_ENTITY,
		CB_ENTITY_COMPONENT
	};

	struct {
		ECopyBufferData dataType = CB_NONE;
		FMemStream data;
		FClass* type = nullptr;
	} copyBuffer;

private:
	CCameraController* camController;

	CPropertyEditor* propertyEditor;
	CConsoleWidget* consoleWidget;
	CInputOutputWidget* ioWidget;
	CProjectSettingsWidget* projSettingsWidget;
	CAssetBrowserWidget* assetBrowser;
	CAddonsWindow* addonsWindow;
	CEditorSettingsWidget* editorSettings;

	CObjectDebugger* objectDebuggerWidget;

	TObjectPtr<CMaterial> outlineMat;

	FMesh boxOutlineMesh;
};

inline CEditorEngine* gEditorEngine() { return (CEditorEngine*)gEngine; }
