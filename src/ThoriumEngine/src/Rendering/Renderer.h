#pragma once

#include "EngineCore.h"
#include "Shader.h"
#include "RenderCommands.h"
#include "Assets/ModelAsset.h"
#include "Assets/TextureAsset.h"
#include "Rendering/RenderProxies.h"
#include "Rendering/Texture.h"
#include "Framebuffer.h"
#include "Buffers.h"
#include "Console.h"
#include <mutex>

#include "Renderer.generated.h"

#define ENABLE_DEFERRED_RENDERING 1

class IRenderer;
class IGraphicsInterface;
class CRenderScene;
class IFrameBuffer;
class IDepthBuffer;
class ISwapChain;
class IBaseWindow;
class ITexture2D;
class ITextureCube;

struct FRenderStatistics;

extern ENGINE_API IRenderer* gRenderer;
extern ENGINE_API IGraphicsInterface* gGHI;

extern ENGINE_API FRenderStatistics gRenderStats;

// 0 = None, 1 = Unlit, 2 = Normal, 3 = Material
extern ENGINE_API CConVar cvRenderMaterialMode;

extern ENGINE_API CConVar cvRenderShadowEnabled;
extern ENGINE_API CConVar cvRenderShadowQuality;
extern ENGINE_API CConVar cvRenderTextureQuality;

extern ENGINE_API CConVar cvRenderScreenPercentage;

// Screen space Ambient Occlusion.
extern ENGINE_API CConVar cvRenderSsaoEnabled;
extern ENGINE_API CConVar cvRenderSsaoQuality;

// Screen space shadows.
extern ENGINE_API CConVar cvRenderSsShadows;
extern ENGINE_API CConVar cvRenderSsShadowsQuality;

// Bloom
extern ENGINE_API CConVar cvRenderBloomEnabled;
extern ENGINE_API CConVar cvRenderBloomIntensity;
extern ENGINE_API CConVar cvRenderBloomThreshold;
extern ENGINE_API CConVar cvRenderBloomKnee;

extern ENGINE_API CConVar cvRenderFBPointFilter;

extern ENGINE_API CConVar cvForceForwardRendering;

enum class EGraphicsApi
{
	NONE,
	DIRECTX_11,
	DIRECTX_12,
	VULKAN,

	DEFAULT,
};

enum class EBlendMode
{
	BLEND_DISABLED,
	BLEND_ADDITIVE,
	BLEND_ADDITIVE_COLOR
};

// DEPRECATED - Moved to using ConVars
//struct FGraphicsSettings
//{
//	uint8 lightingQuality : 2; // Low - Medium - High - Ultra
//	uint8 textureQuality : 2; // Low - Medium - High - Ultra
//	uint8 shadowQuality : 2; // 256x - 512x - 1024x - 2048x
//	uint8 effectsQuality : 2;
//	uint8 renderScale = 100;
//
//	bool bVsync : 1;
//	bool bUseBloom : 1;
//	bool bMotionBlur : 1;
//};

struct FSceneInfoBuffer
{
	FMatrix camMatrix;
	FMatrix camView;
	FMatrix camProjection;
	FMatrix invCamMatrix;
	FMatrix invCamView;
	FMatrix invCamProjection;

	FVector cameraPos;
	int padding_cameraPos;
	FVector cameraDir;
	int padding_cameraDir;

	float time;
	float exposure;
	float gamma;

	float padding_gamma;

	FVector2 framebufferScale;
	FVector2 viewport;
};

struct FObjectInfoBuffer
{
	FMatrix skeletonMatrices[48];
	FMatrix transform;
	FVector position;
	int padding_position;
};

struct FDirectionalLightData
{
	FVector direction;
	float _padding;
	FVector color;
	float intensity;
	FVector _padding2;
	int shadowIndex;
};

struct FPointLightData
{
	FVector position;
	float _padding1;
	FVector color;
	float intensity;
	float _padding2[2];
	float range;
	int shadowIndex;
};

struct FSpotLightData
{
	FVector position;
	float _padding1;
	FVector direction;
	float _padding2;
	FVector color;
	float intensity;
	float innerConeAngle;
	float outerConeAngle;
	float range;
	int shadowIndex;
};

struct FForwardLightsBuffer
{
	FDirectionalLightData dirLights[2];
	FPointLightData pointLights[8];
	FSpotLightData spotLights[8];

	int numDirLights;
	int numPointLights;
	int numSpotLights;
};

struct FShadowDataBuffer
{
	FMatrix vSpotShadowMatrix[8];
	FMatrix vSunShadowMatrix[4];
	FQuaternion vPointShadowPos[8]; // not actually a quat but I need a 16 byte structure

	// -1 if there is none
	int vSpotShadowId[8];
	int vPointShadowId[8];

	float vSpotShadowBias[8];
	float vPointShadowBias[8];

	int vSunShadowId;
	float vSunShadowBias;
};

struct FBloomSettings
{
	float intensity;
	float threshold;
	float knee;
};

struct FTextSDFBuffer
{
	FVector color;
	float _padding;
	FVector outlineColor;
	float _padding2;

	float thickness;
	float outline;

	uint uvStride;
	uint uvIndex;
};

struct FRenderStatistics
{
	SizeType numTris;
	SizeType numDrawCalls;

	SizeType totalPrimitives;
	SizeType drawPrimitives;
};

// -- Buffer Registers --
// 0 : Unused
// 1 : Scene Buffer
// 2 : Scene Lights
// 3 : Object Buffer
// 4 : Lights Shadow Data
// 5 : CubeMap Data
// 6 : Material Buffer

// -- Texture Registers --
// 0 : Spot Light Shadows
// 1 : Point Light Shadows
// 2 : Sun Shadow
// 3 : Prev-Pass Frame Buffer
// 4 : Unused
// 5 : Base Color
// 6 : Normal Map
// 7-20 : Material Textures

CLASS(Abstract)
class ENGINE_API IRenderer : public CObject
{
	friend class IShader;
	friend class CEngine;
	friend class CEditorEngine;

	GENERATED_BODY()

public:
	IRenderer();
	virtual ~IRenderer() = default;

	inline void PushScene(CRenderScene* scene) { renderScenes.Add(scene); }

	/**
	 * Render the frame in a seperate thread.
	 */
	void RenderMT();
	void JoinRenderThread();

	inline void Render() { renderAll(); }

	inline static void LockGPU() { gpuMutex.lock(); }
	inline static void UnlockGPU() { gpuMutex.unlock(); }

	// the scene that is being rendered.
	inline CRenderScene* CurrentScene() const { return curScene; }

	// the camera that is being rendered.
	inline CCameraProxy* CurrentCamera() const { return curCamera; }

public:
	static void Blit(IFrameBuffer* source, IFrameBuffer* destination);
	static void Blit(IFrameBuffer* source, IFrameBuffer* destination, int destinationMip);

	static void Blit(IFrameBuffer* source, IFrameBuffer* destination, FVector2 viewportPos, FVector2 viewportScale);
	static void Blit(IFrameBuffer* source, IFrameBuffer* destination, int destinationMip, FVector2 viewportPos, FVector2 viewportScale);

protected:
	virtual void Init();

	virtual void renderAll();

	virtual void RenderCamera(CRenderScene* scene, CCameraProxy* camera) = 0;
	virtual void RenderShadowMaps(CRenderScene* scene) = 0;
	virtual void RenderUserInterface(CRenderScene* scene) = 0;

	static TIterator<FRenderCommand> __getRenderCommands(ERenderPass currentPass,
		TIterator<FRenderCommand> begin, TIterator<FRenderCommand> end, TArray<FRenderCommand>& out);
	
protected:
	bool bInitialized;

	// the currently rendering scene and camera
	CRenderScene* curScene;
	CCameraProxy* curCamera;

	TArray<CRenderScene*> renderScenes;

	TObjectPtr<CShaderSource> debugUnlit;
	TObjectPtr<CShaderSource> debugNormalForward;

	TObjectPtr<CShaderSource> shaderScreenPlane;
	TObjectPtr<CShaderSource> shaderBlit;

	static std::mutex gpuMutex;
};
