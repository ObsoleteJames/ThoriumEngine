
#include "TextureAsset.h"
#include "Rendering/Renderer.h"
#include "Rendering/Texture.h"
#include "Console.h"
#include <mutex>
#include <chrono>

#include "ThirdParty/stb_image.h"
#include "ThirdParty/stb_image_resize.h"
#include "ThirdParty/stb_dxt.h"
#include "ThirdParty/stb_image_resize.h"

#define THTEX_VERSION 0x0003
#define THTEX_VERSION_01 0x0001
#define THTEX_VERSION_02 0x0002

#define THTEX_MAGIC_SIZE 29
static const char* thtexMagicStr = "\0\0ThoriumEngine Texture File\0";

class CTextureStreamingProxy : public IResourceStreamingProxy
{
public:
	CTextureStreamingProxy(CTexture* t, uint8 mipMap)
	{
		tex = t;
		targetTex = (ITexture2D*)tex->tex;
		targetMipMap = mipMap;
		currentMipMap = t->numMipmaps;
		stream = t->File()->GetStream("rb");
	}

	virtual ~CTextureStreamingProxy()
	{
		tex->bLoading = false;
	}

	void Load() override
	{
		bLoading = true;

		SizeType offset = THTEX_MAGIC_SIZE + sizeof(uint16) + sizeof(ETextureAssetFormat) + 4 + 4 + 1;
		if (tex->version != THTEX_VERSION_01)
			offset += sizeof(ETextureFilter);

		stream->Seek(offset, SEEK_SET);

		if (bDirty)
		{
			bLoading = false;
			return;
		}

		SizeType dataSize = 0;
		for (int i = 0; i < currentMipMap; i++)
		{
			*stream >> &dataSize;
			stream->Seek(dataSize, SEEK_CUR);
		}
		*stream >> &dataSize;

		if (data)
			free(data);
		
		data = (uint8*)malloc(dataSize);
		THORIUM_ASSERT(data, "Failed to allocate memory for texture (" + FString::ToString(dataSize) + " bytes)");

		stream->Read(data, dataSize);
		bDirty = true;
		bLoading = false;

		if (currentMipMap == targetMipMap)
			bFinished = true;
	}

	void PushData() override
	{
		tex->curMipMapLevel = currentMipMap;
		if (data && targetTex)
			targetTex->UpdateData(data, currentMipMap);
		//CONSOLE_LogInfo("Updated Texture with MipMap: " + FString::ToString(currentMipMap));
		
		free(data);
		data = nullptr;

		currentMipMap--;
		bDirty = false;
	}

public:
	TUniquePtr<IBaseFStream> stream;
	TObjectPtr<CTexture> tex;
	ITexture2D* targetTex;
	int8 targetMipMap;
	
	std::atomic<int8> currentMipMap;

	uint8* data = nullptr;

};

static void ConvertImageType(int width, int height, uint8* in, uint8* out, uint inChannels, uint outChannels)
{
	bool bDownSizing = inChannels > outChannels;

	uint texSize = width * height;
	for (uint i = 0; i < texSize; i++)
	{
		for (uint p = 0; p < outChannels; p++)
		{
			if (p < inChannels)
				out[(i * outChannels) + p] = in[(i * inChannels) + p];
			else if (p != 3)
				out[(i * outChannels) + p] = 0;
			else
				out[(i * outChannels) + p] = 255;
		}
	}
}

CTexture::~CTexture()
{
	delete tex;
}

void CTexture::Init()
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CTexture", "Failed to create file stream for '" + ToFString(file->Path()) + "'");
		return;
	}

	char magicStr[THTEX_MAGIC_SIZE];
	stream->Read(magicStr, THTEX_MAGIC_SIZE);

	if (memcmp(thtexMagicStr, magicStr, THTEX_MAGIC_SIZE) != 0)
	{
		CONSOLE_LogError("CTexture", FString("Invalid Texture file '") + ToFString(file->Path()) + "'");
		return;
	}

	version;
	*stream >> &version;

	if (version != THTEX_VERSION && version != THTEX_VERSION_01 && version != THTEX_VERSION_02)
	{
		CONSOLE_LogError("CTexture", FString("Invalid Texture file version '") + FString::ToString(version) + "'  expected version '" + FString::ToString(THTEX_VERSION) + "'");
		return;
	}

	*stream >> &format >> &width >> &height;
	*stream >> &numMipmaps;
	if (version != THTEX_VERSION_01)
		*stream >> &filteringType;

	if (version <= THTEX_VERSION_02 && format >= THTX_FORMAT_RGBA16_FLOAT)
		format = (ETextureAssetFormat)((int)format + 1);

	*stream >> &dataSize;

	curMipMapLevel = numMipmaps;

	if (numMipmaps > 1)
		tex = gRenderer->CreateTexture2D(nullptr, numMipmaps, width, height, ToTextureFormat(format), filteringType);

	bLoading = false;
	bInitialized = true;
}

void CTexture::Init(void* data, int width, int height, ETextureAssetFormat format /*= THTX_FORMAT_RGBA8_UINT*/, ETextureFilter filter)
{
	if (tex)
		delete tex;

	tex = gRenderer->CreateTexture2D(data, width, height, ToTextureFormat(format), filter);
	bInitialized = true;
}

void CTexture::Load(uint8 lodLevel)
{
	if (!file || !bInitialized || curMipMapLevel <= lodLevel || bLoading)
		return;

	if (numMipmaps > 1)
	{
		bLoading = true;

		CResourceManager::StreamResource(new CTextureStreamingProxy(this, lodLevel));
		return;
	}

	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CTexture", "Failed to create file stream for '" + ToFString(file->Path()) + "'");
		return;
	}

	SizeType offset = THTEX_MAGIC_SIZE + sizeof(uint16) + sizeof(format) + 4 + 4 + 1;
	if (version != THTEX_VERSION_01)
		offset += sizeof(filteringType);

	stream->Seek(offset, SEEK_SET);

	stream->Seek(8, SEEK_CUR);
	uint8* data = (uint8*)malloc(dataSize);
	stream->Read(data, dataSize);

	if (tex)
		delete tex;

	tex = gRenderer->CreateTexture2D(data, width, height, ToTextureFormat(format), filteringType);
	curMipMapLevel = 0;
}

void CTexture::Unload(uint8 lodLevel)
{

}

bool CTexture::Import(const FString& file, const FTextureImportSettings& settings)
{
	if (!this->file || bLoading)
		return false;

	//FILE* f;
	//_wfopen_s(&f ,file.c_str(), L"rb");
	
	//if (!f)
	//	return false;

	int numChannels[] = {
		1,
		2,
		4,
		4,
		4,
		4,
		4,
		4,
		4,
		4
	};

	int comp;
	uint8* data = stbi_load(file.c_str(), &width, &height, &comp, numChannels[settings.format]);
	//fclose(f);

	if (!data)
	{
		CONSOLE_LogError("CTexture", "Failed to import texture.");
		return false;
	}

	format = settings.format;

	if (format >= THTX_FORMAT_AUTO)
	{
		if (comp == 1)
			format = THTX_FORMAT_R8_UINT;
		else if (comp == 2)
			format = THTX_FORMAT_RG8_UINT;
		else if (comp == 3)
			format = format == THTX_FORMAT_AUTO ? THTX_FORMAT_RGB8_UINT : THTX_FORMAT_DXT1;
		else if (comp == 4)
			format = format == THTX_FORMAT_AUTO ? THTX_FORMAT_RGBA8_UINT : THTX_FORMAT_DXT1;
	}

	if (format > THTX_FORMAT_RGBA32_FLOAT && (width % 8 != 0 || height % 8 != 0))
	{
		int w = width + (8 - (width % 8));
		int h = height + (8 - (height % 8));

		uint8* newData = (uint8*)malloc(w * h * numChannels[format]);
		if (!stbir_resize_uint8(data, width, height, 0, newData, w, h, 0, numChannels[format]))
		{
			CONSOLE_LogError("CTexture", "Failed to import texture. Compressed textures need width/height to be power of 8.");
			free(data);
			free(newData);
			return false;
		}

		width = w;
		height = h;
		free(data);
		data = newData;
	}

	// { Data, Size } - Highest to lowest
	TArray<TPair<uint8*, SizeType>> mipMaps;

	dataSize = width * height * numChannels[format];
	numMipmaps = settings.numMipMaps;

	for (uint8 i = 0; i < numMipmaps; i++)
	{
		SizeType mipPow = std::pow(2, i);
		SizeType mipSize = i > 0 ? dataSize / mipPow : dataSize;
		uint8* mipData = data;

		if (width / mipPow <= 16)
		{
			numMipmaps = i + 1;
			break;
		}

		if (mipSize != dataSize)
		{
			SizeType newWidth = width / mipPow;
			SizeType newHeight = height / mipPow;
			mipData = (uint8*)malloc(mipSize);
			stbir_resize_uint8(data, width, height, 0, mipData, newWidth, newHeight, 0, numChannels[format]);
		}

		if (format > THTX_FORMAT_RGBA32_FLOAT)
		{
			if (format == THTX_FORMAT_DXT1)
				mipSize /= 8;
			else
				mipSize /= 4;

			SizeType newWidth = width / mipPow;
			SizeType newHeight = height / mipPow;

			uint8* compressedMip = (uint8*)malloc(mipSize);
			rygCompress(compressedMip, mipData, newWidth, newHeight, format == THTX_FORMAT_DXT5);
			if (mipData != data)
				free(mipData);
			mipData = compressedMip;
		}

		mipMaps.Add({ mipData, mipSize });
	}

	//if (format > THTX_FORMAT_RGBA32_FLOAT)
	//{
	//	if (format == THTX_FORMAT_DXT1)
	//		dataSize /= 8;
	//	else
	//		dataSize /= 4;

	//	uint8* compressedTex = (uint8*)malloc(dataSize);

	//	rygCompress(compressedTex, data, width, height, format == THTX_FORMAT_DXT5);
	//	free(data);
	//	data = compressedTex;
	//}

	if (tex)
	{
		delete tex;
		tex = nullptr;
	}

	void** _d = (void**)alloca(sizeof(void*) * numMipmaps);
	for (int i = 0; i < numMipmaps; i++)
		_d[i] = (void*)mipMaps[i].Key;

	tex = gRenderer->CreateTexture2D(_d, numMipmaps, width, height, ToTextureFormat(format), filteringType);
	curMipMapLevel = 0;

	FFile* texFile = this->file;
	TUniquePtr<IBaseFStream> stream = texFile->GetStream("wb");

	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("CTexture", "Failed to create file stream for texture.");
		for (auto& mp : mipMaps)
			free(mp.Key);

		return false;
	}

	stream->Write((void*)thtexMagicStr, THTEX_MAGIC_SIZE);

	uint16 version = THTEX_VERSION;
	*stream << &version;

	*stream << &format;
	*stream << &width << &height;
	//*stream << &dataSize;
	*stream << &numMipmaps;
	*stream << &filteringType;

	for (auto& mp : mipMaps)
	{
		*stream << &mp.Value;
		stream->Write(mp.Key, mp.Value);
	}
	
	for (auto& mp : mipMaps)
		free(mp.Key);

	CFStream sdkStream = texFile->GetSdkStream("wb");
	if (sdkStream.IsOpen())
	{
		sdkStream << file;
		sdkStream.Close();
	}

	bLoading = false;
	bInitialized = true;
	return true;
}

ETextureFormat CTexture::ToTextureFormat(ETextureAssetFormat format)
{
	switch (format)
	{
	case THTX_FORMAT_R8_UINT:
		return TEXTURE_FORMAT_R8_UNORM;
	case THTX_FORMAT_RG8_UINT:
		return TEXTURE_FORMAT_RG8_UNORM;
	case THTX_FORMAT_RGB8_UINT:
		return TEXTURE_FORMAT_RGB8_UNORM;
	case THTX_FORMAT_RGBA8_UINT:
		return TEXTURE_FORMAT_RGBA8_UNORM;

	case THTX_FORMAT_RGBA16_FLOAT:
		return TEXTURE_FORMAT_RGBA16_FLOAT;
	case THTX_FORMAT_RGBA32_FLOAT:
		return TEXTURE_FORMAT_RGBA32_FLOAT;

	case THTX_FORMAT_DXT1:
		return TEXTURE_FORMAT_DXT1;
	case THTX_FORMAT_DXT5:
		return TEXTURE_FORMAT_DXT5;
	}
	return TEXTURE_FORMAT_INVALID;
}

CTexture* CTexture::CreateFromImage(const FString& file)
{
	int width, height, comp;

	uint8* data = stbi_load(file.c_str(), &width, &height, &comp, 0);

	if (!data)
	{
		CONSOLE_LogError("CTexture", "Failed to create texture from image.");
		return nullptr;
	}

	TObjectPtr<CTexture> tex = CreateObject<CTexture>();
	tex->Init(data, width, height, (ETextureAssetFormat)(comp - 1), THTX_FILTER_LINEAR);

	stbi_image_free(data);
	return tex;
}
