
#include <stdint.h>
#include <string>
#include <vector>

#define CLASS(...)

using FRenderPass = uint16_t;
using FString = std::string;

template<typename T>
using TArray = std::vector<T>;

namespace RenderPass
{
	constexpr static FRenderPass DEFERRED = 1;
	constexpr static FRenderPass FORWARD = 2;
	constexpr static FRenderPass POST_PROCESS = 3;

	constexpr static FRenderPass DEBUG = 4;
	constexpr static FRenderPass DEBUG_OVERLAY = 5;

	constexpr static FRenderPass USER_INTERFACE = 6;
}

class ITexture2D;
class ITextureCube;
class IRenderTexture;
class IShaderBuffer;

class IRenderPass;
class ICameraProxy;
class IPrimitiveProxy;

class CRenderScene;
class CShaderSource;

struct FMaterialInstance
{
	FRenderPass renderPass;
	FString debugName;

	bool isTransparent = false;

	CShaderSource* shader;
	IShaderBuffer* buffer;
};

CLASS(Abstract)
class IRenderApi
{
public:
	// The usual api stuff here.
};

class IRenderPass
{
	friend class CRenderer;

public:
	IRenderPass() = default;
	virtual ~IRenderPass() = default;

	// Called after being registered.
	virtual void SetupResources(IRenderApi* api) {}

	// Called every frame.
	virtual void Render(CRenderScene* scene, IRenderApi* api) = 0;

	inline const FString& GetName() const { return name; }
	inline FRenderPass GetId() const { return id; }
	inline FRenderPass GetOrder() const { return order; }

	// The amount of time this render pass took to render. (in ms)
	inline float GetExecutionTime() const { return executionTime; }

protected:
	FString name = "INVALID_RENDERPASS_NAME";

	// The id of this renderpass
	FRenderPass id = -1;

	// This value will determin when this render pass will be executed. 
	// e.g. if this value is POST_PROCESS; it will execute after the PostProcess pass has been executed.
	FRenderPass order;

private:
	float executionTime;

};

class CRenderer
{
public:
	void RenderMT();
	void RenderFrame();

	// Wait for the current frame to finish rendering.
	void AwaitFrame();

	void RenderCamera(ICameraProxy* cam, CRenderScene* scene);
	void RenderShadowMaps(CRenderScene* scene);

	void RegisterRenderPass(IRenderPass* pass);
	void UnregisterRenderPass(IRenderApi* pass);

	inline IRenderApi* GetApi() const { return api; }

private:
	IRenderApi* api;
	TArray<CRenderScene*> scenes;
};
