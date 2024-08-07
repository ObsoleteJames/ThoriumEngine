#pragma once

#include "EngineCore.h"
#include "RenderCommands.h"
#include "Framebuffer.h"
#include "RenderProxies.h"
#include "Game/Components/CameraComponent.h"

class ILightComponent;
class ITexture2D;
class CCameraComponent;
class CPrimitiveProxy;
class CCameraProxy;
class CPostProcessVolumeProxy;

struct FPrimitiveHitInfo
{
	CPrimitiveProxy* hitProxy;
	FVector position;
	FVector normal;
	float distance; // distance from the ray position.
	int hitFace; // index of the face that was hit.
	int materialIndex;
};

class ENGINE_API CRenderScene
{
	friend class IRenderer;

public:
	CRenderScene(int fbWidth = 1280, int fbHeight = 720);
	~CRenderScene();

	//inline void SetCamera(CCameraComponent* newCam) { camera = newCam; }
	//inline TObjectPtr<CCameraComponent> GetCamera() const { return camera; }

	inline int GetFrameBufferWidth() const { return bufferWidth; }
	inline int GetFrameBufferHeight() const { return bufferHeight; }

	inline float GetTime() const { return time; }
	inline void SetTime(float f) { time = f; }

	inline void SetFrameBuffer(IFrameBuffer* target) { frameBuffer = target; }

	UTIL_DEPRECATED("This function has been deprecated")
	inline void SetDepthBuffer(IDepthBuffer* depth) { }

	inline void PushCommand(const FRenderCommand& cmd) { renderQueue.Add(cmd); }
	inline const TArray<FRenderCommand>& GetRenderQueue() const { return renderQueue; }
	
	inline void RegisterPrimitive(CPrimitiveProxy* proxy) { primitives.Add(proxy); }
	inline void UnregisterPrimitve(CPrimitiveProxy* proxy) { if (auto it = primitives.Find(proxy); it != primitives.end()) primitives.Erase(it); }
	inline const TArray<CPrimitiveProxy*>& GetPrimitives() const { return primitives; }
	inline void SetPrimitives(const TArray<CPrimitiveProxy*>& arr) { primitives = arr; }

	inline void RegisterCamera(CCameraProxy* proxy) { cameras.Add(proxy); }
	inline void UnregisterCamera(CCameraProxy* proxy) { if (auto it = cameras.Find(proxy); it != cameras.end()) cameras.Erase(it); }
	inline const TArray<CCameraProxy*>& GetCameras() const { return cameras; }
	inline void SetCameras(const TArray<CCameraProxy*>& arr) { cameras = arr; }

	inline CCameraProxy* GetPrimaryCamera() const { return primaryCamera; }
	inline void SetPrimaryCamera(CCameraProxy* cam) { primaryCamera = cam; }

	inline void RegisterLight(CLightProxy* proxy) { lights.Add(proxy); }
	inline void UnregisterLight(CLightProxy* proxy) { if (auto it = lights.Find(proxy); it != lights.end()) lights.Erase(it); }
	inline const TArray<CLightProxy*>& GetLights() const { return lights; }
	inline void SetLights(const TArray<CLightProxy*>& arr) { lights = arr; }

	inline void RegisterPPVolume(CPostProcessVolumeProxy* proxy) { ppVolumes.Add(proxy); }
	inline void UnregisterPPVolume(CPostProcessVolumeProxy* proxy) { if (auto it = ppVolumes.Find(proxy); it != ppVolumes.end()) ppVolumes.Erase(it); }
	inline const TArray<CPostProcessVolumeProxy*>& GetPostProcessVolumes() const { return ppVolumes; }
	inline void SetPostProcessVolumes(const TArray<CPostProcessVolumeProxy*>& arr) { ppVolumes = arr; }

	inline void RegisterCubeMap(CCubeMapProxy* proxy) { cubemaps.Add(proxy); }
	inline void UnregisterCubeMap(CCubeMapProxy* proxy) { if (auto it = cubemaps.Find(proxy); it != cubemaps.end()) cubemaps.Erase(it); }
	inline const TArray<CCubeMapProxy*>& GetCubeMaps() const { return cubemaps; }
	inline void SetCubeMaps(const TArray<CCubeMapProxy*>& arr) { cubemaps = arr; }

	inline void SetScreenPercentage(float f) { screenPercentage = f; }
	inline float ScreenPercentage() const { return screenPercentage; }

	bool RayCast(const FVector& pos, const FVector& dir, FPrimitiveHitInfo* outHit, float maxDistance = 0.f);
	bool RayCastBounds(const FRay& ray, FPrimitiveHitInfo* outHit, float maxDistance = 0.f);

	void GeneratePrimitiveGraph();

	void ResizeBuffers(int width, int height);

private:
	TArray<FRenderCommand> renderQueue;

	TArray<CPrimitiveProxy*> primitives;
	TArray<CCameraProxy*> cameras;
	TArray<CLightProxy*> lights;
	TArray<CPostProcessVolumeProxy*> ppVolumes;
	TArray<CCubeMapProxy*> cubemaps;

	//TArray<CTexture*> lightmaps;

	CCameraProxy* primaryCamera = nullptr;

	float time;

	float screenPercentage = 100.f;

	int bufferWidth, bufferHeight;

public:
	// the position of the primary camera that was used for rendering the sun light shadow map,
	FVector sunLightCamPos;
	FVector sunLightCamDir;

public:
	IFrameBuffer* colorBuffer; // float16 output buffer
	IFrameBuffer* GBufferA; // Normal
	IFrameBuffer* GBufferB; // Material (red = metallic, green = roughness, blue = ao, alpha = specular)
	IFrameBuffer* GBufferC; // Albedo
	IFrameBuffer* GBufferD; // ?? maybe material type

	// Frame buffer used for things like refraction.
	IFrameBuffer* preTranslucentBuff;

	// Ambient occlusion buffer.
	IFrameBuffer* aoBuffer;

	IFrameBuffer* bloomBuffersX[4];
	IFrameBuffer* bloomBuffersY[4];

	ITexture2D* depthTex;

	IDepthBuffer* depth;

	IFrameBuffer* frameBuffer = nullptr;
};
