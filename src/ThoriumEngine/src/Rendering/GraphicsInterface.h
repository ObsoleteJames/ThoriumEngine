#pragma once

#include "Renderer.h"

extern ENGINE_API IGraphicsInterface* GetGraphicsInterface(EGraphicsApi api = EGraphicsApi::DEFAULT);

class ENGINE_API IGraphicsInterface
{
public:
	virtual ~IGraphicsInterface() = default;
	virtual void Init() = 0;

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
	virtual void CopyResource(ITextureCube* source, ITextureCube* destination) = 0;
	virtual void CopyResource(ITextureCube* source, ITexture2D* destination, int targetFace) = 0;
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

public:
	inline EGraphicsApi GetApi() const { return api; }

protected:
	EGraphicsApi api;

};
