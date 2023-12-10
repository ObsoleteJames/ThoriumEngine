
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Resources/Texture.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_ETextureAssetFormat : public FEnum
{
	public:
	FEnum_ETextureAssetFormat()
	{
		values.Add({ "THTX_FORMAT_R8_UINT", (int64)ETextureAssetFormat::THTX_FORMAT_R8_UINT });
		values.Add({ "THTX_FORMAT_RG8_UINT", (int64)ETextureAssetFormat::THTX_FORMAT_RG8_UINT });
		values.Add({ "THTX_FORMAT_RGB8_UINT", (int64)ETextureAssetFormat::THTX_FORMAT_RGB8_UINT });
		values.Add({ "THTX_FORMAT_RGBA8_UINT", (int64)ETextureAssetFormat::THTX_FORMAT_RGBA8_UINT });
		values.Add({ "THTX_FORMAT_RGBA16_FLOAT", (int64)ETextureAssetFormat::THTX_FORMAT_RGBA16_FLOAT });
		values.Add({ "THTX_FORMAT_RGBA32_FLOAT", (int64)ETextureAssetFormat::THTX_FORMAT_RGBA32_FLOAT });
		values.Add({ "THTX_FORMAT_DXT1", (int64)ETextureAssetFormat::THTX_FORMAT_DXT1 });
		values.Add({ "THTX_FORMAT_DXT5", (int64)ETextureAssetFormat::THTX_FORMAT_DXT5 });
		values.Add({ "THTX_FORMAT_AUTO", (int64)ETextureAssetFormat::THTX_FORMAT_AUTO });
		values.Add({ "THTX_FORMAT_AUTO_COMPRESSED", (int64)ETextureAssetFormat::THTX_FORMAT_AUTO_COMPRESSED });
		name = "TextureAssetFormat";
		cppName = "ETextureAssetFormat";
		size = sizeof(ETextureAssetFormat);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_ETextureAssetFormat __FEnum_ETextureAssetFormat_Instance;

class FEnum_ETextureFilter : public FEnum
{
	public:
	FEnum_ETextureFilter()
	{
		values.Add({ "THTX_FILTER_LINEAR", (int64)ETextureFilter::THTX_FILTER_LINEAR });
		values.Add({ "THTX_FILTER_POINT", (int64)ETextureFilter::THTX_FILTER_POINT });
		values.Add({ "THTX_FILTER_ANISOTROPIC", (int64)ETextureFilter::THTX_FILTER_ANISOTROPIC });
		name = "TextureFilter";
		cppName = "ETextureFilter";
		size = sizeof(ETextureFilter);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_ETextureFilter __FEnum_ETextureFilter_Instance;

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FAssetClass_CTexture_Tags[] {
	{ "Extension", ".thtex" },
	{ "ImportableAs", ".png;.jpg;.tga" },
};
#endif

class FAssetClass_CTexture : public FAssetClass
{
public:
	FAssetClass_CTexture()
	{
		name = "Texture";
		cppName = "CTexture";
		size = sizeof(CTexture);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 2;
		tags = _FAssetClass_CTexture_Tags;
#endif
		extension = ".thtex";
		importableAs = ".png;.jpg;.tga";
		BaseClass = CAsset::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		assetFlags = ASSET_NONE;
		GetModule_Engine().RegisterFClass(this);
		GetModule_Engine().RegisterFAsset(this);
	}
	CObject* Instantiate() override { return new CTexture(); }
};
FAssetClass_CTexture __FAssetClass_CTexture_Instance;

FClass* CTexture::StaticClass() { return &__FAssetClass_CTexture_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
