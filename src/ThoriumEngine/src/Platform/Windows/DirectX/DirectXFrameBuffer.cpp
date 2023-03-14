
#include "DirectXFrameBuffer.h"
#include "Window.h"

DirectXFrameBuffer::DirectXFrameBuffer(ID3D11Texture2D* fromTexture, int w, int h)
{
	width = w;
	height = h;

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateRenderTargetView(fromTexture, nullptr, &targetView);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX framebuffer");
	IRenderer::UnlockGPU();
}

DirectXFrameBuffer::DirectXFrameBuffer(int w, int h, int type)
{
	width = w;
	height = h;

	constexpr DXGI_FORMAT formats[] = {
		DXGI_FORMAT_R8_UINT,
		DXGI_FORMAT_R8G8_UINT,
		DXGI_FORMAT_R8G8B8A8_UINT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};

	IRenderer::LockGPU();
	D3D11_TEXTURE2D_DESC tex{};
	tex.Width = width;
	tex.Height = height;
	tex.MipLevels = 1;
	tex.ArraySize = 1;
	tex.Format = formats[type];
	tex.SampleDesc.Count = 1;
	tex.SampleDesc.Quality = 0;
	tex.Usage = D3D11_USAGE_DEFAULT;
	tex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&tex, nullptr, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX Texture2D");

	hr = GetDirectXRenderer()->device->CreateRenderTargetView(buffer, nullptr, &targetView);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX framebuffer");

	IRenderer::UnlockGPU();
}

DirectXFrameBuffer::~DirectXFrameBuffer()
{
	targetView->Release();
	if (buffer)
		buffer->Release();
}

void DirectXFrameBuffer::Resize(int width, int height)
{

}

void DirectXFrameBuffer::Clear(float r, float g, float b, float a)
{
	float bg[] = { r, g, b, a };
	GetDirectXRenderer()->deviceContext->ClearRenderTargetView(targetView, bg);
}

DirectXDepthBuffer::DirectXDepthBuffer(int width, int height)
{
	D3D11_TEXTURE2D_DESC depthBuffInfo{};
	depthBuffInfo.Width = width;
	depthBuffInfo.Height = height;
	depthBuffInfo.MipLevels = 1;
	depthBuffInfo.ArraySize = 1;
	depthBuffInfo.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBuffInfo.SampleDesc.Count = 1;
	depthBuffInfo.SampleDesc.Quality = 0;
	depthBuffInfo.Usage = D3D11_USAGE_DEFAULT;
	depthBuffInfo.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&depthBuffInfo, nullptr, &depthBuffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX depth buffer");

	hr = GetDirectXRenderer()->device->CreateDepthStencilView(depthBuffer, NULL, &depthView);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX depth view");
}

DirectXDepthBuffer::~DirectXDepthBuffer()
{
	depthBuffer->Release();
	depthView->Release();
}

void DirectXDepthBuffer::Clear(float a)
{
	GetDirectXRenderer()->deviceContext->ClearDepthStencilView(depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, a, 0);
}

DirectXSwapChain::DirectXSwapChain(IBaseWindow* window)
{
	int width, height;
	window->GetSize(width, height);

	DXGI_SWAP_CHAIN_DESC1 swapchainInfo{};
	swapchainInfo.Width = width;
	swapchainInfo.Height = height;
	swapchainInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchainInfo.Stereo = false;
	swapchainInfo.SampleDesc.Count = 1;
	swapchainInfo.SampleDesc.Quality = 0;
	swapchainInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainInfo.BufferCount = 1;
	swapchainInfo.Scaling = DXGI_SCALING_STRETCH;
	swapchainInfo.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchainInfo.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr = GetDirectXRenderer()->factory->CreateSwapChainForHwnd(
		GetDirectXRenderer()->device,
		(HWND)window->GetNativeHandle(),
		&swapchainInfo,
		nullptr,
		nullptr,
		&swapchain);

	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX SwapChain!");

	CreateViewBuffers(width, height);
}

DirectXSwapChain::~DirectXSwapChain()
{
	delete depth;
	delete framebuffer;
	swapchain->Release();
}

void DirectXSwapChain::Present(int bVSync, int flags)
{
	swapchain->Present(bVSync, flags);
}

IFrameBuffer* DirectXSwapChain::GetFrameBuffer()
{
	return framebuffer;
}

IDepthBuffer* DirectXSwapChain::GetDepthBuffer()
{
	return depth;
}

void DirectXSwapChain::Resize(int width, int height)
{
	delete depth;
	delete framebuffer;
	swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);

	CreateViewBuffers(width, height);
}

void DirectXSwapChain::CreateViewBuffers(int w, int h)
{
	ID3D11Texture2D* backBuff;
	HRESULT hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuff);
	framebuffer = new DirectXFrameBuffer(backBuff, w, h);
	backBuff->Release();

	depth = new DirectXDepthBuffer(w, h);
}
