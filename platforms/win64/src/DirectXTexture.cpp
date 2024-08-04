
#include "Platform/Windows/DirectX/DirectXTexture.h"
#include "Console.h"

DirectXTexture2D::DirectXTexture2D(void* data, int w, int h, ETextureFormat f, ETextureFilter filter) : format(f)
{
	type = TextureType_2D;

	width = w;
	height = h;

	D3D11_TEXTURE2D_DESC texd{};
	texd.Width = width;
	texd.Height = height;
	texd.MipLevels = 1;
	texd.ArraySize = 1;
	texd.Format = DirectXInterface::GetDXTextureFormat(format).Key;
	texd.SampleDesc.Count = 1;
	texd.SampleDesc.Quality = 0;
	texd.Usage = data ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texd.CPUAccessFlags = data ? D3D11_CPU_ACCESS_WRITE : 0;
	//texd.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	int pitch = width * DirectXInterface::GetDXTextureFormat(format).Value;

	D3D11_SUBRESOURCE_DATA imgData{};
	imgData.pSysMem = data;
	imgData.SysMemPitch = pitch;

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&texd, data ? &imgData : nullptr, &tex);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX Texture2D");
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
		CONSOLE_LogError("ITexture", "Failed to create DirectX SRV");
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
		CONSOLE_LogError("ITexture", "Failed to create DirectX SamplerState");
		return;
	}

	mipMapCount = 1;
}

DirectXTexture2D::DirectXTexture2D(void** data, int numMimMaps, int w, int h, ETextureFormat f, ETextureFilter filter) : format(f)
{
	type = TextureType_2D;

	width = w;
	height = h;

	D3D11_TEXTURE2D_DESC texd{};
	texd.Width = width;
	texd.Height = height;
	texd.MipLevels = numMimMaps;
	texd.ArraySize = 1;
	texd.Format = DirectXInterface::GetDXTextureFormat(format).Key;
	texd.SampleDesc.Count = 1;
	texd.SampleDesc.Quality = 0;
	texd.Usage = D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA* imgData = new D3D11_SUBRESOURCE_DATA[numMimMaps];;
	if (data)
	{
		for (int i = 0; i < numMimMaps; i++)
		{
			imgData[i].pSysMem = data[i];
			imgData[i].SysMemPitch = (width / std::pow(2, i)) * DirectXInterface::GetDXTextureFormat(format).Value;
		}
		suppliedMipMapData = numMimMaps;
	}
	else
	{
		for (int i = 0; i < numMimMaps; i++)
		{
			SizeType size = (width * height) / std::pow(2, i) * DirectXInterface::GetDXTextureFormat(format).Value;
			imgData[i].pSysMem = malloc(size);
			if (i == numMimMaps - 1)
				memset((void*)imgData[i].pSysMem, 0, size);

			imgData[i].SysMemPitch = (width / std::pow(2, i)) * DirectXInterface::GetDXTextureFormat(format).Value;
		}
	}

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&texd, imgData, &tex);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX Texture2D");
		return;
	}

	if (!data)
	{
		for (int i = 0; i < numMimMaps; i++)
		{
			free((void*)imgData[i].pSysMem);
		}
	}

	delete[] imgData;

	//GetDirectXRenderer()->deviceContext->UpdateSubresource(tex, 0, nullptr, data, pitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texd.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = numMimMaps - 1;
	srvDesc.Texture2D.MipLevels = -1;
	hr = GetDirectXRenderer()->device->CreateShaderResourceView(tex, &srvDesc, &view);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX SRV");
		return;
	}

	//GetDirectXRenderer()->deviceContext->GenerateMips(view);

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = filter == THTX_FILTER_LINEAR ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : (filter == THTX_FILTER_POINT ? D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR : D3D11_FILTER_ANISOTROPIC);
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = numMimMaps;

	hr = GetDirectXRenderer()->device->CreateSamplerState(&samplerDesc, &sampler);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX SamplerState");
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

	GetDirectXRenderer()->deviceContext->UpdateSubresource(tex, mipmapLevel, nullptr, p, (width / std::pow(2, mipmapLevel)) * DirectXInterface::GetDXTextureFormat(format).Value, (height / std::pow(2, mipmapLevel)) * DirectXInterface::GetDXTextureFormat(format).Value);

	//D3D11_MAPPED_SUBRESOURCE data;
	//HRESULT hr = GetDirectXRenderer()->deviceContext->Map(tex, mipmapLevel, D3D11_MAP_WRITE_DISCARD, 0, &data);
	//if (FAILED(hr))
	//	return;
	//memcpy(data.pData, p, (width * height) / std::pow(2, mipmapLevel));
	//GetDirectXRenderer()->deviceContext->Unmap(tex, mipmapLevel);

	if (suppliedMipMapData < mipMapCount - mipmapLevel)
		suppliedMipMapData = mipMapCount - mipmapLevel;

	UpdateView();
}

void DirectXTexture2D::UpdateView()
{
	if (view)
		view->Release();

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DirectXInterface::GetDXTextureFormat(format).Key;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = mipMapCount - suppliedMipMapData;
	srvDesc.Texture2D.MipLevels = -1;
	HRESULT hr = GetDirectXRenderer()->device->CreateShaderResourceView(tex, &srvDesc, &view);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX SRV");
		return;
	}
}

DirectXTextureCube::DirectXTextureCube(void* data, int w, int h, ETextureFormat f, ETextureFilter filter) : format(f)
{
	type = TextureType_Cube;

	width = w;
	height = h;

	D3D11_TEXTURE2D_DESC texd{};
	texd.Width = width;
	texd.Height = height;
	texd.MipLevels = 1;
	texd.ArraySize = 6;
	texd.Format = DirectXInterface::GetDXTextureFormat(f).Key;
	texd.SampleDesc.Count = 1;
	texd.SampleDesc.Quality = 0;
	texd.Usage = D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texd.CPUAccessFlags = 0;

	int pitch = width * DirectXInterface::GetDXTextureFormat(format).Value;

	D3D11_SUBRESOURCE_DATA imgData{};
	imgData.pSysMem = data;
	imgData.SysMemPitch = pitch;

	HRESULT hr = GetDirectXRenderer()->device->CreateTexture2D(&texd, data ? &imgData : nullptr, &tex);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX Texture2D Cube");
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = texd.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	hr = GetDirectXRenderer()->device->CreateShaderResourceView(tex, &srvDesc, &view);
	if (FAILED(hr))
	{
		CONSOLE_LogError("ITexture", "Failed to create DirectX SRV");
		return;
	}

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
		CONSOLE_LogError("ITexture", "Failed to create DirectX SamplerState");
		return;
	}
}

DirectXTextureCube::~DirectXTextureCube()
{

}
