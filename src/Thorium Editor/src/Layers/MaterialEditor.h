#pragma once

#include "Layer.h"
#include "EngineCore.h"
#include "Object/Object.h"

class CWorld;
class CModelEntity;
class CRenderScene;
class CModelAsset;
class CMaterial;
class IFrameBuffer;
class IDepthBuffer;
class CCameraProxy;
class CPointLightComponent;
class CMaterialEditor;

typedef void(CMaterialEditor::* funcSaveCallback)(int result);

class CMaterialEditor : public CLayer
{
public:
	CMaterialEditor();

	void SetMaterial(CMaterial*, bool bNew = false);
	inline CMaterial* GetMaterial() const { return mat; }

protected:
	void OnUpdate(double dt) override;
	void OnUIRender() override;

	void OnDetach() override;

	void SaveMat();

	void DrawProperties();

private:
	void OnSaveOpenMaterial(int result);
	void OnSaveExit(int result);
	void OnSaveNewMaterial(int result);

private:
	bool bWantsToClose = false;
	bool bSaved = false;

	TObjectPtr<CMaterial> mat;
	TObjectPtr<CModelAsset> previewModel;
	TObjectPtr<CModelEntity> modelEnt;

	TObjectPtr<CWorld> scene;

	CCameraProxy* camera;
	CPointLightComponent* light1;
	CPointLightComponent* light2;

	IFrameBuffer* fbView;
	IDepthBuffer* dbView;

	// Dialog IDs
	FString openMatId;
	FString saveMatId;

	float sizeL = 260;
	float sizeR = 200;

	int viewportWidth, viewportHeight;
	float viewportX = 0.f, viewportY = 0.f;

private:
	funcSaveCallback saveCallback = nullptr;

};
