#pragma once

#include "Layer.h"
#include "EngineCore.h"
#include "Object/Object.h"
#include "Resources/ModelAsset.h"

class CWorld;
class CModelEntity;
class CRenderScene;
class CCameraProxy;
class CPointLightComponent;

class IFrameBuffer;
class IDepthBuffer;

class CModelEditor : public CLayer
{
public:
	CModelEditor();

	void SetModel(CModelAsset* mdl);
	inline CModelAsset* GetModel() const { return mdl; }

protected:
	void OnUpdate(double dt) override;
	void OnUIRender() override;

	void OnDetach() override;

private:
	bool bSaved = true;
	bool bExit = false;

	TObjectPtr<CModelAsset> mdl;
	TObjectPtr<CModelEntity> modelEnt;
	TObjectPtr<CWorld> scene;

	CCameraProxy* camera;

	CPointLightComponent* light1;
	
	IFrameBuffer* framebuffer;
	IDepthBuffer* depthbuffer;

	FString openMdlId;
	FString saveMdlId;

	float sizeL = 360;
	float sizeR;

	int viewportWidth = 32, viewportHeight = 32;
	float viewportX = 0.f, viewportY = 0.f;
};
