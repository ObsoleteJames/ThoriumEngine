#pragma once

#include "Asset.h"
#include "TextureAsset.generated.h"

class IBaseTexture;
enum ETextureFormat;

ENUM()
enum ETextureAssetFormat
{
	THTX_FORMAT_R8_UINT,
	THTX_FORMAT_RG8_UINT,
	THTX_FORMAT_RGB8_UINT,
	THTX_FORMAT_RGBA8_UINT,

	THTX_FORMAT_RGBA16_FLOAT,
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
	uint numMipMaps = 12;
	ETextureAssetFormat format = THTX_FORMAT_AUTO_COMPRESSED;
	ETextureFilter filter = THTX_FILTER_ANISOTROPIC;
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

	void Init(void* data, int width, int height, ETextureAssetFormat format = THTX_FORMAT_RGBA8_UINT, ETextureFilter filter = THTX_FILTER_LINEAR);
	virtual void Init();

	virtual void Save() {}
	virtual void Load(uint8 lodLevel);
	virtual void Unload(uint8 lodLevel);

	bool Import(const FString& file, const FTextureImportSettings& settings = FTextureImportSettings());

	inline int GetWidth() const { return width; }
	inline int GetHeight() const { return height; }

	inline ETextureAssetFormat Format() const { return format; }
	inline ETextureFilter FilterType() const { return filteringType; }
	inline uint8 MipMapCount() const { return numMipmaps; }

	inline SizeType DataSize() const { return dataSize; }

	inline IBaseTexture* GetTextureObject() const { return tex; }

	static ETextureFormat ToTextureFormat(ETextureAssetFormat format);

	static CTexture* CreateFromImage(const FString& file);

protected:
	uint16 version;

	ETextureAssetFormat format;
	ETextureFilter filteringType = THTX_FILTER_ANISOTROPIC;
	SizeType dataSize = 0;
	uint8 numMipmaps = 0;

	uint8 curMipMapLevel = 0;
	uint8 bLoading = false;

	int width, height;

	IBaseTexture* tex = nullptr;

};
