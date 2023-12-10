#pragma once

#include "Rendering/Renderer.h"

enum ETextureFormat
{
	TEXTURE_FORMAT_INVALID,
	TEXTURE_FORMAT_R8_UNORM,
	TEXTURE_FORMAT_RG8_UNORM,
	TEXTURE_FORMAT_RGB8_UNORM,
	TEXTURE_FORMAT_RGBA8_UNORM,

	TEXTURE_FORMAT_R10G10B10A2_UNORM,
	TEXTURE_FORMAT_R11G11B10_FLOAT,

	TEXTURE_FORMAT_RGBA16_FLOAT,
	TEXTURE_FORMAT_RGBA32_FLOAT,
	TEXTURE_FORMAT_DXT1,
	TEXTURE_FORMAT_DXT5
};


class ENGINE_API ITexture2D
{
public:
	virtual ~ITexture2D() = default;

	virtual void UpdateData(void* data, int mipmapLevel) = 0;

};

class ENGINE_API ITextureCube
{
public:
	virtual ~ITextureCube() = default;

};
