#pragma once

#include "DirectXInterface.h"
#include "Rendering/Texture.h"
#include "Assets/TextureAsset.h"

class ENGINE_API DirectXTexture2D : public ITexture2D
{
public:
	DirectXTexture2D(const FTextureDescriptor& desc);
	virtual ~DirectXTexture2D();

	void UpdateData(void* data, int mipmapLevel, int slice) override;

	void Map(FMappedResource* outData, EResourceMapType type, int mip, int slice) override;
	void Unmap(int mip, int slice) override;

	void UpdateView();

	D3D11_SHADER_RESOURCE_VIEW_DESC CreateViewDesc(int mipLevel);

public:
	ID3D11Texture2D* tex = nullptr;
	ID3D11ShaderResourceView* view = nullptr;
	ID3D11SamplerState* sampler = nullptr;
	
	int suppliedMipMapData = 0;
};

//class ENGINE_API DirectXTextureCube : public ITextureCube
//{
//public:
//	DirectXTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter);
//	virtual ~DirectXTextureCube();
//
//public:
//	ID3D11Texture2D* tex = nullptr;
//	ID3D11ShaderResourceView* view = nullptr;
//	ID3D11SamplerState* sampler = nullptr;
//
//	ETextureFormat format;
//
//};
