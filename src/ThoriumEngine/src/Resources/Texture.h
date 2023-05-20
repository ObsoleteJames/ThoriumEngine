#pragma once

#include "Asset.h"
#include "Texture.generated.h"

class ITexture2D;

ENUM()
enum ETextureFormat
{
	THTX_FORMAT_R8_UINT,
	THTX_FORMAT_RG8_UINT,
	THTX_FORMAT_RGB8_UINT,
	THTX_FORMAT_RGBA8_UINT,

	THTX_FORMAT_RGBA32_FLOAT,
	THTX_FORMAT_DXT1,
	THTX_FORMAT_DXT5,

	THTX_FORMAT_AUTO,
	THTX_FORMAT_AUTO_COMPRESSED
};

ENUM()
enum ETextureFilter
{
	THTX_FILTER_LINEAR,
	THTX_FILTER_POINT,
	THTX_FILTER_ANISOTROPIC
};

struct FTextureImportSettings
{
	uint numMipMaps = 5;
	ETextureFormat format;
	ETextureFilter filter;
};

ASSET(Extension = ".thtex", ImportableAs = ".png;.jpg;.tga")
class ENGINE_API CTexture : public CAsset
{
	GENERATED_BODY()

	friend class CTextureStreamingProxy;

public:
	CTexture() = default;
	//CTexture(void* data, int width, int height, ETextureFormat format = THTX_FORMAT_RGBA8_UINT);
	virtual ~CTexture();

	void Init(void* data, int width, int height, ETextureFormat format = THTX_FORMAT_RGBA8_UINT, ETextureFilter filter = THTX_FILTER_LINEAR);
	virtual void Init();

	virtual void Save() {}
	virtual void Load(uint8 lodLevel);
	virtual void Unload(uint8 lodLevel);

	bool Import(const WString& file, const FTextureImportSettings& settings = FTextureImportSettings());

	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }

	inline ITexture2D* GetTextureObject() const { return tex; }

private:
	uint16 version;

	ETextureFormat format;
	ETextureFilter filteringType = THTX_FILTER_ANISOTROPIC;
	SizeType dataSize;
	uint8 numMipmaps;

	uint8 curMipMapLevel;
	uint8 bLoading;

	int width, height;

	ITexture2D* tex = nullptr;

};
