
#include "Platform/Windows/DirectX/DirectXFrameBuffer.h"
#include "Window.h"
#include "Console.h"

DirectXFrameBuffer::DirectXFrameBuffer(ID3D11Texture2D* fromTexture, int w, int h)
{
	type = TextureType_Framebuffer;

	width = w;
	height = h;

	IRenderer::LockGPU();
	HRESULT hr = GetDirectXRenderer()->device->CreateRenderTargetView(fromTexture, nullptr, &targetView);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX framebuffer");
	IRenderer::UnlockGPU();
}

DirectXFrameBuffer::DirectXFrameBuffer(int w, int h, ETextureFormat f, ETextureFilter _filter) : filter(_filter)
{
	type = TextureType_Framebuffer;
	Generate(w, h, f);
}

DirectXFrameBuffer::~DirectXFrameBuffer()
{
	targetView->Release();
	if (buffer)
		buffer->Release();
}

void DirectXFrameBuffer::Resize(int width, int height)
{
	if (targetView)
		targetView->Release();
	if (buffer)
		buffer->Release();

	if (view)
		view->Release();
	if (sampler)
		sampler->Release();

	Generate(width, height, format);
}

void DirectXFrameBuffer::Clear(float r, float g, float b, float a)
{
	float bg[] = { r, g, b, a };
	GetDirectXRenderer()->deviceContext->ClearRenderTargetView(targetView, bg);
}

void DirectXFrameBuffer::Generate(int w, int h, ETextureFormat f)
{
	width = w;
	height = h;
	format = f;

	//THORIUM_ASSERT(format < THTX_FORMAT_DXT1, "Invalid format for framebuffer");

	IRenderer::LockGPU();
	D3D11_TEXTURE2D_DESC tex{};
	tex.Width = width;
	tex.Height = height;
	tex.MipLevels = 1;
	tex.ArraySize = 1;
	tex.Format = DirectXRenderer::GetDXTextureFormat(format).Key;
	tex.SampleDesc.Count = 1;
	tex.SampleDesc.Quality = 0;
	tex.Usage = D3D11_USAGE_DEFAULT;
	tex.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&tex, nullptr, &buffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX Texture2D");

	hr = GetDirectXRenderer()->device->CreateRenderTargetView(buffer, nullptr, &targetView);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX framebuffer");

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = tex.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	hr = GetDirectXRenderer()->device->CreateShaderResourceView(buffer, &srvDesc, &view);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX SRV");
		return;
	}

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = filter == THTX_FILTER_LINEAR ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

	hr = GetDirectXRenderer()->device->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX SamplerState");
		return;
	}

	IRenderer::UnlockGPU();
}

DirectXDepthBuffer::DirectXDepthBuffer(FDepthBufferInfo data)
{
	info = data;

	Generate();
}

DirectXDepthBuffer::~DirectXDepthBuffer()
{
	if (depthView)
		depthView->Release();
	if (depthBuffer)
		depthBuffer->Release();
	if (view)
		view->Release();
	if (sampler)
		sampler->Release();
}

void DirectXDepthBuffer::Resize(int w, int h)
{
	if (w == info.width && h == info.height)
		return;

	if (depthView)
		depthView->Release();
	if (depthBuffer)
		depthBuffer->Release();
	if (view)
		view->Release();
	if (sampler)
		sampler->Release();

	info.width = w;
	info.height = h;
	Generate();
}

void DirectXDepthBuffer::Clear(float a)
{
	GetDirectXRenderer()->deviceContext->ClearDepthStencilView(depthView, info.format == TH_DBF_D24_S8 ? D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL : D3D11_CLEAR_DEPTH, a, 0);
}

void DirectXDepthBuffer::Generate()
{
	D3D11_TEXTURE2D_DESC depthBuffInfo{};
	depthBuffInfo.Width = info.width;
	depthBuffInfo.Height = info.height;
	depthBuffInfo.MipLevels = 1;
	depthBuffInfo.ArraySize = info.arraySize;
	depthBuffInfo.Format = info.format == TH_DBF_D24_S8 ? DXGI_FORMAT_D24_UNORM_S8_UINT : (info.format == TH_DBF_R32 ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_R16_TYPELESS);
	//depthBuffInfo.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthBuffInfo.SampleDesc.Count = 1;
	depthBuffInfo.SampleDesc.Quality = 0;
	depthBuffInfo.Usage = D3D11_USAGE_DEFAULT;
	depthBuffInfo.BindFlags = info.bShaderResource ? D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE : D3D11_BIND_DEPTH_STENCIL;

	IRenderer::LockGPU();

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&depthBuffInfo, nullptr, &depthBuffer);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX depth buffer");

	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc{};
	depthViewDesc.Format = info.format == TH_DBF_D24_S8 ? DXGI_FORMAT_D24_UNORM_S8_UINT : (info.format == TH_DBF_R32 ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D16_UNORM);
	depthViewDesc.ViewDimension = info.arraySize > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;
	if (info.arraySize > 1)
	{
		D3D11_TEX2D_ARRAY_DSV tdsv{};
		tdsv.ArraySize = info.arraySize;
		tdsv.FirstArraySlice = 0;
		tdsv.MipSlice = 0;
		depthViewDesc.Texture2DArray = tdsv;
	}
	else
		depthViewDesc.Texture2D.MipSlice = 0;

	hr = GetDirectXRenderer()->device->CreateDepthStencilView(depthBuffer, &depthViewDesc, &depthView);
	THORIUM_ASSERT(SUCCEEDED(hr), "Failed to create DirectX depth view");

	if (info.bShaderResource)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = info.format == TH_DBF_D24_S8 ? DXGI_FORMAT_D24_UNORM_S8_UINT : (info.format == TH_DBF_R32 ? DXGI_FORMAT_R32_FLOAT : DXGI_FORMAT_R16_UNORM);
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		hr = GetDirectXRenderer()->device->CreateShaderResourceView(depthBuffer, &srvDesc, &view);
		if (FAILED(hr))
		{
			CONSOLE_LogError("ITexture", "Failed to create DirectX SRV");
		}

		D3D11_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

		hr = GetDirectXRenderer()->device->CreateSamplerState(&samplerDesc, &sampler);
		if (FAILED(hr))
		{
			CONSOLE_LogError("ITexture", "Failed to create DirectX SamplerState");
		}
	}

	IRenderer::UnlockGPU();
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

	FDepthBufferInfo depthInfo{};
	depthInfo.width = w;
	depthInfo.height = h;
	depthInfo.arraySize = 1;
	depthInfo.format = TH_DBF_D24_S8;

	depth = new DirectXDepthBuffer(depthInfo);
}
