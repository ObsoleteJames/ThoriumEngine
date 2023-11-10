#pragma once

#include "Rendering/Renderer.h"
#include <d3d11.h>
#include <dxgi1_2.h>
#pragma comment(lib, "d3d11.lib")

class DirectXRenderer;
extern DirectXRenderer* GetDirectXRenderer();

class ENGINE_API DirectXRenderer : public IRenderer
{
public:
	DirectXRenderer() = default;
	virtual ~DirectXRenderer();

	virtual void CompileShader(const FString& source, IShader::EType shaderType, void** outBuffer, SizeType* outBufferSize);
	
	virtual IShader* GetVsShader(CShaderSource* shader);
	virtual IShader* GetPsShader(CShaderSource* shader);

	virtual IVertexBuffer* CreateVertexBuffer(const TArray<FVertex>& vertices);
	virtual IIndexBuffer* CreateIndexBuffer(const TArray<uint>& indices);

	virtual IShaderBuffer* CreateShaderBuffer(void* data, SizeType size);

	virtual ISwapChain* CreateSwapChain(IBaseWindow* window);
	virtual IDepthBuffer* CreateDepthBuffer(FDepthBufferInfo depthInfo);
	virtual IFrameBuffer* CreateFrameBuffer(int width, int height, ETextureFormat format);

	virtual ITexture2D* CreateTexture2D(void* data, int width, int height, ETextureFormat format, ETextureFilter filter);
	virtual ITexture2D* CreateTexture2D(void** data, int numMipMaps, int width, int height, ETextureFormat format, ETextureFilter filter);
	virtual ITextureCube* CreateTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter) { return nullptr; }

	//virtual void BindGlobalData();

	virtual void DrawMesh(FMesh* mesh);
	virtual void DrawMesh(FDrawMeshCmd* info);
	virtual void DrawMesh(FMeshBuilder::FRenderMesh* mesh);

	virtual void SetVsShader(IShader* shader);
	virtual void SetPsShader(IShader* shader);

	virtual void SetShaderBuffer(IShaderBuffer* buffer, int _register);

	virtual void SetShaderResource(ITexture2D* texture, int _register);
	virtual void SetShaderResource(IDepthBuffer* depthTex, int _register);

	virtual void SetFrameBuffer(IFrameBuffer* framebuffer, IDepthBuffer* depth);
	virtual void SetFrameBuffers(IFrameBuffer** framebuffers, SizeType count, IDepthBuffer* depth);

	virtual void SetViewport(float x, float y, float width, float height);

	virtual void SetBlendMode(EBlendMode mode);

	virtual void BindGBuffer();

	virtual void Present();

	static TPair<DXGI_FORMAT, int> GetDXTextureFormat(ETextureFormat format);

	virtual void InitImGui(IBaseWindow* wnd);
	virtual void ImGuiShutdown();
	virtual void ImGuiBeginFrame();
	virtual void ImGuiRender();

	//virtual void Resize(int width, int height);

protected:
	virtual void Init();

public:
	int width, height;

	bool bImGuiGlfw;

	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	IDXGIFactory2* factory;

	ID3D11DepthStencilState* depthStateOff;
	ID3D11DepthStencilState* depthStateReadOnly;

	//ID3D11BlendState* blendDisabled;
	ID3D11BlendState* blendAdditive;

	//IDXGISwapChain* swapchain;
	//ID3D11RenderTargetView* renderTarget;

};
