#pragma once

#include "EngineCore.h"
#include "Rendering/Renderer.h"

#include <vulkan/vulkan.h>

class ENGINE_API VulkanRenderer : public IRenderer
{
public:
	VulkanRenderer() = default;
	virtual ~VulkanRenderer();

	void CompileShader(const FString& file, IShader::EType shaderType, void** outbuffer, SizeType* outBufferSize) final {}

	IShader* GetVsShader(CShaderSource*) final;
	IShader* GetPsShader(CShaderSource*) final;

	IVertexBuffer* CreateVertexBuffer(const TArray<FVertex>&) final;
	IIndexBuffer* CreateIndexBuffer(const TArray<uint>&) final;

	IShaderBuffer* CreateShaderBuffer(void*, SizeType) final;

	ISwapChain* CreateSwapChain(IBaseWindow* window) final;
	IDepthBuffer* CreateDepthBuffer(FDepthBufferInfo) final;
	IFrameBuffer* CreateFrameBuffer(int w, int h, ETextureFormat) final;

	ITexture2D* CreateTexture2D(void* data, int width, int height, ETextureFormat format, ETextureFilter filter) final;
	ITexture2D* CreateTexture2D(void** data, int numMipMaps, int width, int height, ETextureFormat format, ETextureFilter filter) final;
	ITextureCube* CreateTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter) final { return nullptr; }

	void DrawMesh(FMesh*) final;
	void DrawMesh(FDrawMeshCmd*) final;
	void DrawMesh(FMeshBuilder::FRenderMesh*) final;

	void SetVsShader(IShader* shader) final;
	void SetPsShader(IShader* shader) final;

	void SetShaderBuffer(IShaderBuffer*, int r) final;

	void SetShaderResource(ITexture2D* texture, int _register) final;
	void SetShaderResource(IFrameBuffer* fb, int _register) final;
	void SetShaderResource(IDepthBuffer* depthTex, int _register) final;

	void SetFrameBuffer(IFrameBuffer* framebuffer, IDepthBuffer* depth) final;
	void SetFrameBuffers(IFrameBuffer** framebuffers, SizeType count, IDepthBuffer* depth) final;

	void SetViewport(float x, float y, float width, float height) final;

	void SetBlendMode(EBlendMode mode) final;

	void BindGBuffer() final {}

	void Present() final;

	void InitImGui(IBaseWindow* wnd) final;
	void ImGuiShutdown() final;
	void ImGuiBeginFrame() final;
	void ImGuiRender() final;

protected:
	void Init() final;

private:
	VkInstance instance;
	VkDevice device;

};
