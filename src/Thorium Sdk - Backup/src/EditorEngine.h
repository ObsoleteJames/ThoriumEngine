#pragma once

#include "Engine.h"
#include "Rendering/RenderCommands.h"

class CMaterial;
class CCameraComponent;
class CEditorEngine;

struct FMesh;

inline CEditorEngine* gEditorEngine() { return (CEditorEngine*)gEngine; }

class CEditorEngine : public CEngine
{
public:
	void Init() override;

	// Instead of running a loop in here, this gets called by the Qt event loop.
	int Run() override;

	virtual void OnExit() { CEngine::OnExit(); }

public:
	inline void SetRenderScene(CRenderScene* scene) { worldRenderScene = scene; }
	inline void SetDeltaTime(double dt) { deltaTime = dt; }

public:
	CCameraComponent* editorCamera;
	CMaterial* toolsMat;
	FMesh* planeMesh;
};
