#pragma once

#include "Renderer.h"
#include "DefaultRenderer.generated.h"

CLASS()
class ENGINE_API CDefaultRenderer : public IRenderer
{
	GENERATED_BODY()

protected:
	virtual void Init();

	virtual void RenderCamera(CRenderScene* scene, CCameraProxy* camera);
	virtual void RenderShadowMaps(CRenderScene* scene);
	virtual void RenderUserInterface(CRenderScene* scene);

private:
	void PreDeferredLightSetup(CRenderScene* scene);

protected:
	TObjectPtr<CShaderSource> shaderDeferredDirLight;
	TObjectPtr<CShaderSource> shaderDeferredPointLight;
	TObjectPtr<CShaderSource> shaderDeferredSpotLight;

	TObjectPtr<CShaderSource> shaderPPExposure;
	TObjectPtr<CShaderSource> shaderGaussianBlurV;
	TObjectPtr<CShaderSource> shaderGaussianBlurH;
	TObjectPtr<CShaderSource> shaderBloomPreFilter;
	TObjectPtr<CShaderSource> shaderBloomPass;

	TObjectPtr<IGBuffer> sceneBuffer;
	TObjectPtr<IGBuffer> objectBuffer;
	TObjectPtr<IGBuffer> forwardLightsBuffer;
	TObjectPtr<IGBuffer> shadowDataBuffer;
	TObjectPtr<IGBuffer> bloomInfoBuffer;

	TObjectPtr<IGBuffer> deferredLightBuffer;

	TObjectPtr<CModelAsset> meshIcoSphere;

	int shadowTexSize;

	IDepthBuffer* sunLightShadows;
	IDepthBuffer* pointLightShadows;
	IDepthBuffer* spotLightShadows;

	FMesh* quadMesh;
};
