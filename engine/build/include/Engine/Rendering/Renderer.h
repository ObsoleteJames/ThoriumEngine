#pragma once

#include "EngineCore.h"
#include "Shader.h"
#include "RenderCommands.h"
#include "Resources/ModelAsset.h"
#include "Resources/Texture.h"
#include "Rendering/RenderProxies.h"
#include "Rendering/Texture.h"
#include "Framebuffer.h"
#include "Buffers.h"

#include <mutex>

class IRenderer;
class CRenderScene;
class IFrameBuffer;
class IDepthBuffer;
class ISwapChain;
class IBaseWindow;
class ITexture2D;
class ITextureCube;

extern ENGINE_API IRenderer* gRenderer;

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

	FVector cameraPos;
	int padding_cameraPos;
	FVector cameraDir;
	int padding_cameraDir;

	float time;

	FVector2 framebufferScale;
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

constexpr SizeType off = offsetof(FShadowDataBuffer, vSpotShadowId);

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

	virtual IShader* GetVsShader(CShaderSource* shader) = 0;
	virtual IShader* GetPsShader(CShaderSource* shader) = 0;

	virtual IVertexBuffer* CreateVertexBuffer(const TArray<FVertex>& vertices) = 0;
	virtual IIndexBuffer* CreateIndexBuffer(const TArray<uint>& indices) = 0;

	virtual IShaderBuffer* CreateShaderBuffer(void* data, SizeType size) = 0;

	virtual ISwapChain* CreateSwapChain(IBaseWindow* window) = 0;
	virtual IDepthBuffer* CreateDepthBuffer(FDepthBufferInfo depthInfo) = 0;
	virtual IFrameBuffer* CreateFrameBuffer(int width, int height, ETextureFormat format) = 0;

	virtual ITexture2D* CreateTexture2D(void* data, int width, int height, ETextureFormat format, ETextureFilter filter) = 0;
	virtual ITexture2D* CreateTexture2D(void** data, int numMipMaps, int width, int height, ETextureFormat format, ETextureFilter filter) = 0;
	virtual ITextureCube* CreateTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter) = 0;

	virtual void DrawMesh(FMesh* mesh) = 0;
	virtual void DrawMesh(FDrawMeshCmd* info) = 0;
	virtual void DrawMesh(FMeshBuilder::FRenderMesh* mesh) = 0;

	virtual void SetVsShader(IShader* shader) = 0;
	virtual void SetPsShader(IShader* shader) = 0;

	virtual void SetShaderBuffer(IShaderBuffer* buffer, int _register) = 0;

	virtual void SetShaderResource(ITexture2D* texture, int _register) = 0;
	virtual void SetShaderResource(IFrameBuffer* fb, int _register) = 0;
	virtual void SetShaderResource(IDepthBuffer* depthTex, int _register) = 0;

	virtual void SetFrameBuffer(IFrameBuffer* framebuffer, IDepthBuffer* depth = nullptr) = 0;
	virtual void SetFrameBuffers(IFrameBuffer** framebuffers, SizeType count, IDepthBuffer* depth = nullptr) = 0;

	virtual void SetViewport(float x, float y, float width, float height) = 0;

	virtual void SetBlendMode(EBlendMode mode) = 0;

	virtual void BindGBuffer() = 0;

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

private:
	static void renderAll();

	//static void RenderSpotLightShadow(CLightProxy* light, ITexture2D* out);
	//static void RenderSunLightShadow(CLightProxy* light, ITexture2D* out);
	//static void RenderSpotLightShadow(CLightProxy* light, ITextureCube* out);

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

	TObjectPtr<IShaderBuffer> sceneBuffer;
	TObjectPtr<IShaderBuffer> objectBuffer;
	TObjectPtr<IShaderBuffer> forwardLightsBuffer;
	TObjectPtr<IShaderBuffer> forwardShadowDataBuffer;

	//IFrameBuffer* gBuffers[4];

	//IFrameBuffer* ssaoBuffer;

	int shadowTexSize;

	IDepthBuffer* sunLightShadows;
	IDepthBuffer* pointLightShadows;
	IDepthBuffer* spotLightShadows;

	FMesh* quadMesh;

	IShader* vsShaderShadow;
	IShader* psShaderDeferred;
	IShader* vsShaderDeferredLighting;
	IShader* psShaderDeferredLighting;

	static std::mutex gpuMutex;
};

class ENGINE_API Renderer
{
public:
	Renderer() = delete;

	template<typename T>
	static void CreateRenderer();

	static inline IShader* GetVsShader(CShaderSource* shader) { return gRenderer->GetVsShader(shader); }
	static inline IShader* GetPsShader(CShaderSource* shader) { return gRenderer->GetPsShader(shader); }

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
