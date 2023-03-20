
#include "DirectXTexture.h"
#include "Console.h"

static constexpr DXGI_FORMAT formats[] = {
	DXGI_FORMAT_R8_UNORM,
	DXGI_FORMAT_R8G8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R8G8B8A8_UNORM,
	DXGI_FORMAT_R32G32B32A32_FLOAT,
	DXGI_FORMAT_BC1_UNORM,
	DXGI_FORMAT_BC3_UNORM
};

static constexpr int formatSizes[] = {
	1,
	2,
	3,
	4,
	16,
	2,
	4
};

DirectXTexture2D::DirectXTexture2D(void* data, int width, int height, ETextureFormat f, ETextureFilter filter) : format(f)
{
	D3D11_TEXTURE2D_DESC texd{};
	texd.Width = width;
	texd.Height = height;
	texd.MipLevels = 1;
	texd.ArraySize = 1;
	texd.Format = formats[format];
	texd.SampleDesc.Count = 1;
	texd.SampleDesc.Quality = 0;
	texd.Usage = D3D11_USAGE_IMMUTABLE;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texd.CPUAccessFlags = 0;
	//texd.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	int pitch = width * formatSizes[format];

	D3D11_SUBRESOURCE_DATA imgData{};
	imgData.pSysMem = data;
	imgData.SysMemPitch = pitch;

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&texd, &imgData, &tex);
	if (FAILED(hr))
	{
		CONSOLE_LogError("Failed to create DirectX Texture2D");
		return;
	}

	//GetDirectXRenderer()->deviceContext->UpdateSubresource(tex, 0, nullptr, data, pitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texd.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	hr = GetDirectXRenderer()->device->CreateShaderResourceView(tex, &srvDesc, &view);
	if (FAILED(hr))
	{
		CONSOLE_LogError("Failed to create DirectX SRV");
		return;
	}

	//GetDirectXRenderer()->deviceContext->GenerateMips(view);

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = filter == THTX_FILTER_LINEAR ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

	hr = GetDirectXRenderer()->device->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(hr))
	{
		CONSOLE_LogError("Failed to create DirectX SamplerState");
		return;
	}

	mipMapCount = 1;
}

DirectXTexture2D::DirectXTexture2D(void** data, int numMimMaps, int width, int height, ETextureFormat f, ETextureFilter filter) : format(f)
{
	D3D11_TEXTURE2D_DESC texd{};
	texd.Width = width;
	texd.Height = height;
	texd.MipLevels = numMimMaps;
	texd.ArraySize = 1;
	texd.Format = formats[format];
	texd.SampleDesc.Count = 1;
	texd.SampleDesc.Quality = 0;
	texd.Usage = D3D11_USAGE_DYNAMIC;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texd.CPUAccessFlags = 0;
	//texd.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	//int pitch = width * formatSizes[format];

	//D3D11_SUBRESOURCE_DATA imgData{};
	//imgData.pSysMem = data;
	//imgData.SysMemPitch = pitch;

	D3D11_SUBRESOURCE_DATA* imgData = nullptr;
	if (data)
	{
		imgData = new D3D11_SUBRESOURCE_DATA[numMimMaps];
		for (int i = 0; i < numMimMaps; i++)
		{
			imgData[i].pSysMem = data[i];
			imgData[i].SysMemPitch = (width / std::pow(2, i + 1)) * formatSizes[format];
		}
		suppliedMipMapData = numMimMaps;
	}

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&texd, imgData, &tex);
	if (FAILED(hr))
	{
		CONSOLE_LogError("Failed to create DirectX Texture2D");
		return;
	}

	//GetDirectXRenderer()->deviceContext->UpdateSubresource(tex, 0, nullptr, data, pitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texd.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	hr = GetDirectXRenderer()->device->CreateShaderResourceView(tex, &srvDesc, &view);
	if (FAILED(hr))
	{
		CONSOLE_LogError("Failed to create DirectX SRV");
		return;
	}

	//GetDirectXRenderer()->deviceContext->GenerateMips(view);

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = filter == THTX_FILTER_LINEAR ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

	hr = GetDirectXRenderer()->device->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(hr))
	{
		CONSOLE_LogError("Failed to create DirectX SamplerState");
		return;
	}

	mipMapCount = numMimMaps;
}

DirectXTexture2D::~DirectXTexture2D()
{
	if (tex)
		tex->Release();
	if (view)
		view->Release();
	if (sampler)
		sampler->Release();
}

void DirectXTexture2D::UpdateData(void* p, int mipmapLevel)
{
	if (mipmapLevel >= mipMapCount)
		return;

	D3D11_MAPPED_SUBRESOURCE data;
	HRESULT hr = GetDirectXRenderer()->deviceContext->Map(tex, mipmapLevel, D3D11_MAP_WRITE_DISCARD, 0, &data);
	if (FAILED(hr))
		return;
	memcpy(data.pData, p, data.DepthPitch * data.RowPitch);
	GetDirectXRenderer()->deviceContext->Unmap(tex, mipmapLevel);

	if (suppliedMipMapData < mipMapCount - mipmapLevel)
		suppliedMipMapData = mipMapCount - mipmapLevel;

	UpdateView();
}

void DirectXTexture2D::UpdateView()
{
	if (view)
		view->Release();

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = formats[format];
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = mipMapCount - suppliedMipMapData;
	srvDesc.Texture2D.MipLevels = -1;
	HRESULT hr = GetDirectXRenderer()->device->CreateShaderResourceView(tex, &srvDesc, &view);
	if (FAILED(hr))
	{
		CONSOLE_LogError("Failed to create DirectX SRV");
		return;
	}
}

DirectXTextureCube::DirectXTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter)
{

}

DirectXTextureCube::~DirectXTextureCube()
{

}
