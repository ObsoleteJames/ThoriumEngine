#pragma once

#include "EngineCore.h"
#include "Shader.h"
#include "RenderCommands.h"
#include "Resources/ModelAsset.h"
#include "Resources/TextureAsset.h"
#include "Rendering/RenderProxies.h"
#include "Rendering/Texture.h"
#include "Framebuffer.h"
#include "Buffers.h"
#include "Console.h"

#include <mutex>

#define ENABLE_DEFERRED_RENDERING 1

class IRenderer;
class CRenderScene;
class IFrameBuffer;
class IDepthBuffer;
class ISwapChain;
class IBaseWindow;
class ITexture2D;
class ITextureCube;

struct FRenderStatistics;

extern ENGINE_API IRenderer* gRenderer;
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

extern ENGINE_API CConVar cvRenderFBPointFilter;

extern ENGINE_API CConVar cvForceForwardRendering;

enum class ERendererApi
{
	NONE,
	DIRECTX_11,
	DIRECTX_12,
	VULKAN
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

class ENGINE_API IRenderer
{
	friend class IShader;
	friend class Renderer;
	friend class CEngine;

public:
	IRenderer();
	virtual ~IRenderer() = default;

	void BeginRender();

	//inline void PushCommand(const FRenderCommand& cmd) { renderQueue.Add(cmd); }
	inline void PushScene(CRenderScene* scene) { renderScenes.Add(scene); }

	/**
	 * Render the frame in a seperate thread.
	 */
	void RenderMT();
	void JoinRenderThread();

	inline void Render() { renderAll(); }

	inline static void LockGPU() { gpuMutex.lock(); }
	inline static void UnlockGPU() { gpuMutex.unlock(); }

	inline ERendererApi GetApi() const { return api; }

	// the scene that is being rendered.
	inline CRenderScene* CurrentScene() const { return curScene; }

	// the camera that is being rendered.
	inline CCameraProxy* CurrentCamera() const { return curCamera; }

public:
	virtual void CompileShader(const FString& source, IShader::EType shaderType, void** outBuffer, SizeType* outBufferSize) = 0;

	virtual IShader* LoadShader(CShaderSource* source, EShaderType type, FString file) = 0;

	virtual IVertexBuffer* CreateVertexBuffer(const TArray<FVertex>& vertices) = 0;
	virtual IIndexBuffer* CreateIndexBuffer(const TArray<uint>& indices) = 0;

	virtual IShaderBuffer* CreateShaderBuffer(void* data, SizeType size) = 0;

	virtual ISwapChain* CreateSwapChain(IBaseWindow* window) = 0;
	virtual IDepthBuffer* CreateDepthBuffer(FDepthBufferInfo depthInfo) = 0;
	virtual IFrameBuffer* CreateFrameBuffer(int width, int height, ETextureFormat format, ETextureFilter filter = THTX_FILTER_LINEAR) = 0;
	virtual IFrameBuffer* CreateFrameBuffer(int width, int height, int numMipMaps, ETextureFormat format, ETextureFilter filter = THTX_FILTER_LINEAR) = 0;

	virtual ITexture2D* CreateTexture2D(void* data, int width, int height, ETextureFormat format, ETextureFilter filter) = 0;
	virtual ITexture2D* CreateTexture2D(void** data, int numMipMaps, int width, int height, ETextureFormat format, ETextureFilter filter) = 0;
	virtual ITextureCube* CreateTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter) = 0;

	virtual void CopyResource(ITexture2D* source, ITexture2D* destination) = 0;
	virtual void CopyResource(IFrameBuffer* source, ITexture2D* destination) = 0;
	virtual void CopyResource(IFrameBuffer* source, IFrameBuffer* destination) = 0;
	virtual void CopyResource(IDepthBuffer* source, ITexture2D* destination) = 0;
	virtual void CopyResource(IDepthBuffer* source, IFrameBuffer* destination) = 0;

	virtual void DrawMesh(FMesh* mesh) = 0;
	virtual void DrawMesh(FDrawMeshCmd* info) = 0;
	virtual void DrawMesh(FMeshBuilder::FRenderMesh* mesh) = 0;

	// Binds all material data to the current context.
	virtual void SetMaterial(CMaterial* mat) = 0;

	virtual void SetVsShader(IShader* shader) = 0;
	virtual void SetPsShader(IShader* shader) = 0;

	virtual void SetShaderBuffer(IShaderBuffer* buffer, int _register) = 0;

	virtual void SetShaderResource(IBaseTexture* texture, int _register) = 0;
	virtual void SetShaderResource(IDepthBuffer* depthTex, int _register) = 0;

	// OBSOLETE
	//virtual void SetShaderResource(IFrameBuffer* fb, int _register) = 0;

	virtual void SetFrameBuffer(IFrameBuffer* framebuffer, IDepthBuffer* depth = nullptr) = 0;
	virtual void SetFrameBuffer(IFrameBuffer* framebuffer, int mip, IDepthBuffer* depth = nullptr) = 0;
	virtual void SetFrameBuffers(IFrameBuffer** framebuffers, SizeType count, IDepthBuffer* depth = nullptr) = 0;

	virtual void SetViewport(float x, float y, float width, float height) = 0;

	virtual void SetBlendMode(EBlendMode mode) = 0;

	virtual void SetFaceCulling(bool bBack) = 0;

	virtual void Present() = 0;

	virtual void InitImGui(IBaseWindow* wnd) = 0;
	virtual void ImGuiShutdown() = 0;
	virtual void ImGuiBeginFrame() = 0;
	virtual void ImGuiRender() = 0;

	//virtual void Resize(int width, int height) = 0;

protected:
	virtual void Init();
	//virtual void BindGlobalData() = 0;

	static void Blit(IFrameBuffer* source, IFrameBuffer* destination);
	static void Blit(IFrameBuffer* source, IFrameBuffer* destination, int destinationMip);

	static void Blit(IFrameBuffer* source, IFrameBuffer* destination, FVector2 viewportPos, FVector2 viewportScale);
	static void Blit(IFrameBuffer* source, IFrameBuffer* destination, int destinationMip, FVector2 viewportPos, FVector2 viewportScale);

private:
	static void renderAll();

	//static void RenderSpotLightShadow(CLightProxy* light, ITexture2D* out);
	//static void RenderSunLightShadow(CLightProxy* light, ITexture2D* out);
	//static void RenderSpotLightShadow(CLightProxy* light, ITextureCube* out);

	static void PreDeferredLightSetup(CRenderScene* scene);

	static void RenderCamera(CRenderScene* scene, CCameraProxy* camera);
	static void RenderShadowMaps(CRenderScene* scene);
	static void RenderUserInterface(CRenderScene* scene);

	static TIterator<FRenderCommand> __getRenderCommands(ERenderPass currentPass,
		TIterator<FRenderCommand> begin, TIterator<FRenderCommand> end, TArray<FRenderCommand>& out);
	
protected:
	ERendererApi api;

	// the currently rendering scene and camera
	CRenderScene* curScene;
	CCameraProxy* curCamera;

	TArray<CRenderScene*> renderScenes;
	//TArray<FRenderCommand> renderQueue;

	TObjectPtr<CShaderSource> debugUnlit;
	TObjectPtr<CShaderSource> debugNormalForward;

	TObjectPtr<CShaderSource> shaderScreenPlane;
	TObjectPtr<CShaderSource> shaderBlit;

	TObjectPtr<CShaderSource> shaderDeferredDirLight;
	TObjectPtr<CShaderSource> shaderDeferredPointLight;
	TObjectPtr<CShaderSource> shaderDeferredSpotLight;

	TObjectPtr<CShaderSource> shaderPPExposure;
	TObjectPtr<CShaderSource> shaderGaussianBlurV;
	TObjectPtr<CShaderSource> shaderGaussianBlurH;
	TObjectPtr<CShaderSource> shaderBloomPreFilter;
	TObjectPtr<CShaderSource> shaderBloomPass;

	TObjectPtr<IShaderBuffer> sceneBuffer;
	TObjectPtr<IShaderBuffer> objectBuffer;
	TObjectPtr<IShaderBuffer> forwardLightsBuffer;
	TObjectPtr<IShaderBuffer> shadowDataBuffer;

	TObjectPtr<IShaderBuffer> deferredLightBuffer;

	TObjectPtr<CModelAsset> meshIcoSphere;

	//IFrameBuffer* gBuffers[4];

	//IFrameBuffer* ssaoBuffer;

	int shadowTexSize;

	IDepthBuffer* sunLightShadows;
	IDepthBuffer* pointLightShadows;
	IDepthBuffer* spotLightShadows;

	FMesh* quadMesh;

	//IShader* vsShaderShadow;
	//IShader* psShaderDeferred;
	//IShader* vsShaderDeferredLighting;
	//IShader* psShaderDeferredLighting;

	static std::mutex gpuMutex;
};

class ENGINE_API Renderer
{
public:
	Renderer() = delete;

	template<typename T>
	static void CreateRenderer();

	//static inline IShader* GetVsShader(CShaderSource* shader) { return gRenderer->GetVsShader(shader); }
	//static inline IShader* GetPsShader(CShaderSource* shader) { return gRenderer->GetPsShader(shader); }

	static inline IVertexBuffer* CreateVertexBuffer(const TArray<FVertex>& vertices) { return gRenderer->CreateVertexBuffer(vertices); }
	static inline IIndexBuffer* CreateIndexBuffer(const TArray<uint>& indices) { return gRenderer->CreateIndexBuffer(indices); }

	static inline IShaderBuffer* CreateShaderBuffer(void* data, SizeType size) { return gRenderer->CreateShaderBuffer(data, size); }

};

template<typename T>
void Renderer::CreateRenderer()
{
	static_assert(std::is_base_of<IRenderer, T>::value);
	new T();
	gRenderer->Init();
}
