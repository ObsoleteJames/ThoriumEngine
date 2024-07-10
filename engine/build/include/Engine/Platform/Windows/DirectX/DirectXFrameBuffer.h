#pragma once

#include "Rendering/Framebuffer.h"
#include "DirectXRenderer.h"

class CWindow;

class ENGINE_API DirectXFrameBuffer : public IFrameBuffer
{
public:
	DirectXFrameBuffer(ID3D11Texture2D* fromTexture, int w = 0, int h = 0);
	DirectXFrameBuffer(int width, int height, ETextureFormat format, ETextureFilter filter);
	DirectXFrameBuffer(int width, int height, int mipmapCount, ETextureFormat format, ETextureFilter filter);
	virtual ~DirectXFrameBuffer();

	virtual void Resize(int width, int height);
	virtual void Clear(float r, float g, float b, float a);

	inline ID3D11RenderTargetView*& Get(int index = 0) { return targetViews[index]; }

private:
	void Generate(int w, int h, int mipmap, ETextureFormat format);

public:
	ETextureFormat format;
	ETextureFilter filter;

	ID3D11RenderTargetView* targetViews[6] = {};
	ID3D11Texture2D* buffer = nullptr;

	ID3D11ShaderResourceView* view = nullptr;
	ID3D11SamplerState* sampler = nullptr;

	int numMipMaps = 1;

};

class ENGINE_API DirectXDepthBuffer : public IDepthBuffer
{
public:
	DirectXDepthBuffer(FDepthBufferInfo data);
	virtual ~DirectXDepthBuffer();

	virtual void Resize(int width, int height) override;
	virtual void Clear(float a);

	inline ID3D11DepthStencilView*& Get() { return depthView; }
	
private:
	void Generate();

public:
	ID3D11DepthStencilView* depthView = nullptr;
	ID3D11Texture2D* depthBuffer = nullptr;

	ID3D11ShaderResourceView* view = nullptr;
	ID3D11SamplerState* sampler = nullptr;

};

class ENGINE_API DirectXSwapChain : public ISwapChain
{
public:
	DirectXSwapChain(IBaseWindow* window);
	virtual ~DirectXSwapChain();

	virtual void Present(int bVSync, int flags);
	virtual IFrameBuffer* GetFrameBuffer();
	virtual IDepthBuffer* GetDepthBuffer();
	virtual void Resize(int width, int height);

	void CreateViewBuffers(int w, int h);

private:
	IDXGISwapChain1* swapchain = nullptr;
	DirectXDepthBuffer* depth = nullptr;
	DirectXFrameBuffer* framebuffer = nullptr;

};
