#pragma once

#include "DirectXRenderer.h"
#include "Rendering/Texture.h"
#include "Assets/TextureAsset.h"

class ENGINE_API DirectXTexture2D : public ITexture2D
{
public:
	DirectXTexture2D(void* data, int width, int height, ETextureFormat format, ETextureFilter filter);
	DirectXTexture2D(void** data, int numMimMaps, int width, int height, ETextureFormat format, ETextureFilter filter);
	virtual ~DirectXTexture2D();

	void UpdateData(void* data, int mipmapLevel) override;

	void UpdateView();

public:
	ID3D11Texture2D* tex;
	ID3D11ShaderResourceView* view;
	ID3D11SamplerState* sampler;
	ETextureFormat format;
	
	int mipMapCount;
	int suppliedMipMapData = 0;
};

class ENGINE_API DirectXTextureCube : public ITextureCube
{
public:
	DirectXTextureCube(void* data, int width, int height, ETextureFormat format, ETextureFilter filter);
	virtual ~DirectXTextureCube();

public:
	ID3D11Texture3D* tex;
	ID3D11ShaderResourceView* view;
	ID3D11SamplerState* sampler;

};
