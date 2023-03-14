#pragma once

#include "EngineCore.h"
#include "RenderCommands.h"
#include "Framebuffer.h"
#include "RenderProxies.h"
#include "Game/Components/CameraComponent.h"

class ILightComponent;
class CCameraComponent;
class CPrimitiveProxy;
class CCameraProxy;

class ENGINE_API CRenderScene
{
	friend class IRenderer;

public:
	inline void SetCamera(CCameraComponent* newCam) { camera = newCam; }
	inline TObjectPtr<CCameraComponent> GetCamera() const { return camera; }

	inline float GetTime() const { return time; }
	inline void SetTime(float f) { time = f; }

	inline void SetFrameBuffer(IFrameBuffer* target) { frameBuffer = target; }
	inline void SetDepthBuffer(IDepthBuffer* depth) { depthBuffer = depth; }

	inline void PushCommand(const FRenderCommand& cmd) { renderQueue.Add(cmd); }
	
	inline void RegisterPrimitive(CPrimitiveProxy* proxy) { primitves.Add(proxy); }
	inline void UnregisterPrimitve(CPrimitiveProxy* proxy) { if (auto it = primitves.Find(proxy); it != primitves.end()) primitves.Erase(it); }
	inline const TArray<CPrimitiveProxy*>& GetPrimitives() const { return primitves; }
	inline void SetPrimitives(const TArray<CPrimitiveProxy*>& arr) { primitves = arr; }

	inline void RegisterCamera(CCameraProxy* proxy) { cameras.Add(proxy); }
	inline void UnregisterCamera(CCameraProxy* proxy) { if (auto it = cameras.Find(proxy); it != cameras.end()) cameras.Erase(it); }
	inline const TArray<CCameraProxy*>& GetCameras() const { return cameras; }
	inline void SetCameras(const TArray<CCameraProxy*>& arr) { cameras = arr; }

	inline void RegisterLight(CLightProxy* proxy) { lights.Add(proxy); }
	inline void UnregisterLight(CLightProxy* proxy) { if (auto it = lights.Find(proxy); it != lights.end()) lights.Erase(it); }
	inline const TArray<CLightProxy*>& GetLights() const { return lights; }
	inline void SetLights(const TArray<CLightProxy*>& arr) { lights = arr; }

private:
	TArray<FRenderCommand> renderQueue;
	TObjectPtr<CCameraComponent> camera;

	TArray<CPrimitiveProxy*> primitves;
	TArray<CCameraProxy*> cameras;
	TArray<CLightProxy*> lights;

	float time;

	IFrameBuffer* frameBuffer;
	IDepthBuffer* depthBuffer;
};