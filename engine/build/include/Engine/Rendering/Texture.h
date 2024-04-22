#pragma once

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
	TEXTURE_FORMAT_DXT5,

	TEXTURE_FORMAT_R24G8
};

enum ETextureType
{
	TextureType_2D,
	TextureType_Cube,
	TextureType_Framebuffer
};

class ENGINE_API IBaseTexture
{
public:
	virtual ~IBaseTexture() = default;

	inline ETextureType Type() const { return type; }

	inline void GetSize(int& w, int& h) const { w = width; h = height; }

protected:
	ETextureType type;

	int width = 0;
	int height = 0;
};

class ENGINE_API ITexture2D : public IBaseTexture
{
public:
	virtual ~ITexture2D() = default;

	virtual void UpdateData(void* data, int mipmapLevel) = 0;

};

class ENGINE_API ITextureCube : public IBaseTexture
{
public:
	virtual ~ITextureCube() = default;

};
