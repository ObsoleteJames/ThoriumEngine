
#include "Texture.h"
#include "Rendering/Renderer.h"
#include "Rendering/Texture.h"
#include "Console.h"

#include "ThirdParty/stb_image.h"
#include "ThirdParty/stb_image_resize.h"
#include "ThirdParty/stb_dxt.h"
#include "ThirdParty/stb_image_resize.h"

#define THTEX_VERSION 0x0001

#define THTEX_MAGIC_SIZE 29
static const char* thtexMagicStr = "\0\0ThoriumEngine Texture File\0";

//CTexture::CTexture(void* data, int width, int height, ETextureFormat f) : format(f)
//{
//	tex = gRenderer->CreateTexture2D(data, width, height, format);
//	bInitialized = true;
//}

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
		CONSOLE_LogError("Failed to create file stream for '" + ToFString(file->Path()) + "'");
		return;
	}

	char magicStr[THTEX_MAGIC_SIZE];
	stream->Read(magicStr, THTEX_MAGIC_SIZE);

	if (memcmp(thtexMagicStr, magicStr, THTEX_MAGIC_SIZE) != 0)
	{
		CONSOLE_LogError(FString("Invalid Texture file '") + ToFString(file->Path()) + "'");
		return;
	}

	uint16 version;
	*stream >> &version;

	if (version != THTEX_VERSION)
	{
		CONSOLE_LogError(FString("Invalid Texture file version '") + FString::ToString(version) + "'  expected version '" + FString::ToString(THTEX_VERSION) + "'");
		return;
	}

	*stream >> &format >> &width >> &height;
	*stream >> &dataSize;
	*stream >> &numMipmaps;

	bInitialized = true;
}

void CTexture::Init(void* data, int width, int height, ETextureFormat format /*= THTX_FORMAT_RGBA8_UINT*/, ETextureFilter filter)
{
	if (tex)
		delete tex;

	tex = gRenderer->CreateTexture2D(data, width, height, format, filter);
	bInitialized = true;
}

void CTexture::Load(uint8 lodLevel)
{
	if (IsLodLoaded(lodLevel) || !file)
		return;

	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("Failed to create file stream for '" + ToFString(file->Path()) + "'");
		return;
	}
	constexpr SizeType offset = THTEX_MAGIC_SIZE + sizeof(uint16) + sizeof(format) + 4 + 4 + 8 + 1;

	stream->Seek(offset, SEEK_SET);

	uint8* data = (uint8*)malloc(dataSize);
	stream->Read(data, dataSize);

	if (tex)
		delete tex;

	tex = gRenderer->CreateTexture2D(data, width, height, format, filteringType);
	SetLodLevel(lodLevel, true);
}

void CTexture::Unload(uint8 lodLevel)
{

}

bool CTexture::Import(const WString& file, const FTextureImportSettings& settings)
{
	if (!this->file)
		return false;

	FILE* f;
	_wfopen_s(&f ,file.c_str(), L"rb");
	
	if (!f)
		return false;

	int numChannels[] = {
		1,
		2,
		4,
		4,
		4,
		4,
		4,
		0,
		4
	};

	int comp;
	uint8* data = stbi_load_from_file(f, &width, &height, &comp, numChannels[settings.format]);
	fclose(f);

	if (!data)
	{
		CONSOLE_LogError("Failed to import texture.");
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
			CONSOLE_LogError("Failed to import texture. Compressed textures need width/height to be power of 8.");
			free(data);
			free(newData);
			return false;
		}

		width = w;
		height = h;
		free(data);
		data = newData;
	}

	//if (comp != numChannels[format])
	//{
	//	uint8* newData = (uint8*)malloc(width * height * numChannels[format]);
	//	ConvertImageType(width, height, data, newData, comp, numChannels[format]);
	//	free(data);
	//	data = newData;

	//	/*CONSOLE_LogError("Failed to import texture. expected '" + FString::ToString(numChannels[format]) + "' channels got '" + FString::ToString(comp) + "'");
	//	free(data);
	//	return false;*/
	//}

	dataSize = width * height * numChannels[format];

	if (format > THTX_FORMAT_RGBA32_FLOAT)
	{
		if (format == THTX_FORMAT_DXT1)
			dataSize /= 8;
		else
			dataSize /= 4;

		uint8* compressedTex = (uint8*)malloc(dataSize);

		rygCompress(compressedTex, data, width, height, format == THTX_FORMAT_DXT5);
		free(data);
		data = compressedTex;
	}

	numMipmaps = settings.numMipMaps;

	FFile* texFile = this->file;
	TUniquePtr<IBaseFStream> stream = texFile->GetStream("wb");

	if (!stream || !stream->IsOpen())
	{
		CONSOLE_LogError("Failed to create file stream for texture.");
		free(data);
		return false;
	}

	stream->Write((void*)thtexMagicStr, THTEX_MAGIC_SIZE);

	uint16 version = THTEX_VERSION;
	*stream << &version;

	*stream << &format;
	*stream << &width << &height;
	*stream << &dataSize;
	*stream << &numMipmaps;

	// TODO: MimMaps
	stream->Write(data, dataSize);

	free(data);
	return true;
}
