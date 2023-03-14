#pragma once

#include "Rendering/Framebuffer.h"
#include "DirectXRenderer.h"

class CWindow;

class ENGINE_API DirectXFrameBuffer : public IFrameBuffer
{
public:
	DirectXFrameBuffer(ID3D11Texture2D* fromTexture, int w = 0, int h = 0);
	DirectXFrameBuffer(int width, int height, int type);
	virtual ~DirectXFrameBuffer();

	virtual void Resize(int width, int height);
	virtual void Clear(float r, float g, float b, float a);

	inline ID3D11RenderTargetView*& Get() { return targetView; }

public:
	int type;
	ID3D11RenderTargetView* targetView;
	ID3D11Texture2D* buffer;

};

class ENGINE_API DirectXDepthBuffer : public IDepthBuffer
{
public:
	DirectXDepthBuffer(int width, int height);
	virtual ~DirectXDepthBuffer();

	virtual void Clear(float a);

	inline ID3D11DepthStencilView*& Get() { return depthView; }
	
public:
	ID3D11DepthStencilView* depthView;
	ID3D11Texture2D* depthBuffer;

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
	IDXGISwapChain1* swapchain;
	DirectXDepthBuffer* depth;
	DirectXFrameBuffer* framebuffer;

};
