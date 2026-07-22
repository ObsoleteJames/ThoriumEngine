
#include "Font.h"
#include "Console.h"
#include "Rendering/GraphicsInterface.h"
#include "Assets/TextureAsset.h"

#include "ft2build.h"
#include <freetype/ftmodapi.h>
#include <freetype/ftstroke.h>
#include FT_FREETYPE_H

#include "ThirdParty/stb_image_write.h"

static const char* defaultCharecterSet = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
static FString characterSet = defaultCharecterSet;

const char* get_ft_error_string(FT_Error err) {
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       default: return "Unknown FreeType error"; }
#include FT_ERRORS_H
}

#define FT_CheckError(error, msg) if (error) { CONSOLE_LogError("FFont", FString(msg) + " '" + get_ft_error_string(error) + "'"); return; }
#define FT_CheckErrorB(error, msg) if (error) { CONSOLE_LogError("FFont", FString(msg) + " '" + get_ft_error_string(error) + "'"); continue; }

uint32 DecodeUTF8(const char* inCharacter, int& outSize)
{
	char c1 = inCharacter[0];

	// check if this is an ASCII character
	if ((c1 & 0b10000000) == 0)
		return c1;

	// check if this is the leading byte.
	if ((c1 & 0b11000000) == 0b10000000)
		return -1;
	
	uint32 r;
	if ((c1 & 0b1100000) == 0b1100000)
	{
		outSize = 2;
		char c2 = inCharacter[1];
		if ((c1 & 0b1110000) == 0b1110000)
		{
			outSize = 3;
			char c3 = inCharacter[2];
			if ((c1 & 0b1111100) == 0b1111000)
			{
				outSize = 4;
				char c4 = inCharacter[3];
				r = ((c4 & 0b00000111) << 18) | ((c3 & 0b00111111) << 12) | ((c2 & 0b00111111) << 6) | (c1 & 0b00111111);
			}
			else
				r = ((c3 & 0b00001111) << 12) | ((c2 & 0b00111111) << 6) | (c1 & 0b00111111);
		}
		else
			r = ((c1 & 0b00011111) << 6) | (c2 & 0b00111111);
	}

	return r;
}

FFont::FFont(const FString& f, const FFontSettings& s) : settings(s)
{
	FT_Library lib;
	FT_Error err;
	FT_Face face;

	err = FT_Init_FreeType(&lib);
	FT_CheckError(err, "Failed to load FreeType Library!");

	err = FT_New_Face(lib, f.c_str(), 0, &face);
	FT_CheckError(err, "An error occured when creating FreeType Face!");

	err = FT_Set_Pixel_Sizes(face, 0, settings.fontSize);
	FT_CheckError(err, "FreeType Error:");

	if (s.bSDF)
		FT_Property_Set(lib, "sdf", "spread", &s.SDFspread);

	//TArray<TPair<uint32, FT_Glyph>> glyphs;

	//for (auto it = characterSet.begin(); it != characterSet.end(); it++)
	//{
	//	char ch = it[0];

	//	int utfSize = 1;
	//	uint32 utf8Char = DecodeUTF8(it, utfSize);
	//	it = it + (utfSize - 1);

	//	err = FT_Load_Char(face, utf8Char, FT_LOAD_DEFAULT | (s.bAntialiasing ? 0 : FT_LOAD_TARGET_MONO));
	//	FT_CheckErrorB(err, "FreeType Error:");

	//	//err = FT_Render_Glyph(face->glyph, settings.bSDF ? FT_RENDER_MODE_SDF : FT_RENDER_MODE_NORMAL);
	//	//FT_CheckErrorB(err, "FreeType Error:");

	//	FT_Glyph source;
	//	FT_Glyph glyph;
	//	FT_Get_Glyph(face->glyph, &source);
	//	FT_Glyph_Copy(source, &glyph);

	//	glyphs.Add({ utf8Char, glyph });
	//}

	glyphHeight = face->size->metrics.height >> 6;
	int glyphDimensions = glyphHeight + 4;
	if (s.bSDF)
		glyphDimensions += s.SDFspread * 2;

	int minSize = FMath::Ceil(FMath::Sqrt(characterSet.Size())) * glyphDimensions;
	uint64 imgSize = 1;
	while (imgSize < minSize)
		imgSize <<= 1;

	float imgSizeF = (float)imgSize;
	atlasSize = imgSize;

	imgData = (uint8*)malloc(imgSize * imgSize);
	memset(imgData, 0, imgSize * imgSize);
	int curX = 1, curY = 1;

	int largestHeight = 0;

	// Generate atlas
	//for (auto& g : glyphs)
	//{
	for (auto it = characterSet.begin(); it != characterSet.end(); it++)
	{
		char ch = it[0];

		int utfSize = 1;
		uint32 utf8Char = DecodeUTF8(it, utfSize);
		it = it + (utfSize - 1);

		err = FT_Load_Char(face, utf8Char, FT_LOAD_DEFAULT | (s.bAntialiasing ? 0 : FT_LOAD_TARGET_MONO));
		FT_CheckErrorB(err, "FreeType Error:");

		err = FT_Render_Glyph(face->glyph, settings.bSDF ? FT_RENDER_MODE_SDF : FT_RENDER_MODE_NORMAL);
		FT_CheckErrorB(err, "FreeType Error:");

		FT_GlyphSlot g = face->glyph;
		FT_Bitmap& bitmap = face->glyph->bitmap;

		int advance = g->advance.x >> 6;

		/*if (settings.bSDF)
			err = FT_Glyph_To_Bitmap(&g.Value, FT_RENDER_MODE_SDF, nullptr, 1);
		else
			err = FT_Glyph_To_Bitmap(&g.Value, settings.bAntialiasing ? FT_RENDER_MODE_NORMAL : FT_RENDER_MODE_MONO, nullptr, 1);
		FT_CheckErrorB(err, "FreeType Error:");*/

		//FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)g.Value;
		//FT_Bitmap& bitmap = bitmapGlyph->bitmap;

		if (largestHeight < bitmap.rows)
			largestHeight = bitmap.rows;

		if (curX + bitmap.width > imgSize)
		{
			curX = 1;
			curY += largestHeight + 1;
		}

		THORIUM_ASSERT((curY + largestHeight) < imgSize, "Atlas for font is too small to contain all glyphs, cursor went out of bounds!");

		this->glyphs[utf8Char].width = bitmap.width;
		this->glyphs[utf8Char].height = bitmap.rows;
		this->glyphs[utf8Char].uvOffset = FVector2((float)curX / imgSizeF, (float)curY / imgSizeF);
		this->glyphs[utf8Char].uvScale = FVector2((float)bitmap.width / imgSizeF, (float)bitmap.rows / imgSizeF);
		this->glyphs[utf8Char].bearingX = g->metrics.horiBearingX >> 6;
		this->glyphs[utf8Char].bearingY = g->metrics.horiBearingY >> 6;
		this->glyphs[utf8Char].advance = advance;

		for (int r = 0; r < bitmap.rows; r++) // Row
		{
			for (int c = 0; c < bitmap.width; c++) // Column
			{
				int x = curX + c;
				int y = curY + r;

				if (!settings.bAntialiasing)
				{
					int row = r * bitmap.pitch;
					uint8 byte = bitmap.buffer[row + (c / 8)];
					imgData[y * imgSize + x] = (byte & (0x80 >> (c % 8))) != 0 ? 255 : 0;
				}
				else
					imgData[y * imgSize + x] = ((uint8*)bitmap.buffer)[r * bitmap.pitch + c];

			}
		}

		curX += bitmap.width + 1;
	}
	FT_Done_FreeType(lib);

	fontTexture = CreateObject<CTexture>();
	fontTexture->Init(imgData, imgSize, imgSize, THTX_FORMAT_R8_UINT);
}

FFont::~FFont()
{
	free(imgData);
}

void FFont::GenerateMesh(TArray<FVertex>& outVerts, const char* text, const FVector2& bounds, int lineSpacing, int characterSpacing)
{
	int posX = 0;
	int posY = 0;

	for (; text[0] != '\0'; text++)
	{
		int utfSize = 1;
		uint32 utf8Char = DecodeUTF8(text, utfSize);
		text = text + (utfSize - 1);

		if (utf8Char == '\n')
		{
			posX = 0;
			posY += settings.fontSize + lineSpacing;
			continue;
		}
		if (utf8Char == ' ')
		{
			posX += settings.fontSize + characterSpacing;
			continue;
		}
		if (utf8Char == '\t')
		{
			posX += (settings.fontSize + characterSpacing) * 4;
			continue;
		}

		auto it = glyphs.find(utf8Char);
		if (it == glyphs.end())
			continue; // unkown character.

		FGlyph& glyph = it->second;

		int x = posX + glyph.bearingX;
		int y = (posY - glyph.height) + glyphHeight;
		int& w = glyph.width;
		int& h = glyph.height;

		// advance the cursor
		posX += (glyph.advance) + characterSpacing;

		FVector positions[] = {
			FVector( x,		y + h,	0 ),
			FVector( x,		y,		0 ),
			FVector( x + w,	y,		0 ),
			FVector( x,		y + h,	0 ),
			FVector( x + w,	y,		0 ),
			FVector( x + w,	y + h,	0 )
		};

		FVector2& uvMin = glyph.uvOffset;
		FVector2 uvMax = glyph.uvOffset + glyph.uvScale;

		FVector2 uvs[] = {
			{ uvMin.x, uvMax.y },
			{ uvMin.x, uvMin.y },
			{ uvMax.x, uvMin.y },

			{ uvMin.x, uvMax.y },
			{ uvMax.x, uvMin.y },
			{ uvMax.x, uvMax.y },
		};

		for (int i = 0; i < 6; i++)
			outVerts.Add({ positions[i], {/*normal*/}, {/*tangent*/}, {/*color*/}, { uvs[i].x, uvs[i].y }, {0,0}});
	}
}

FVector2 FFont::CalculateTextSize(const char* text) const
{
	return FVector2();
}

void FFont::SavePng(const FString& out)
{
	char* png_data = (char*)malloc(atlasSize * atlasSize * 4);
	for (int i = 0; i < (atlasSize * atlasSize); ++i) {
		png_data[i * 4 + 0] |= imgData[i];
		png_data[i * 4 + 1] |= imgData[i];
		png_data[i * 4 + 2] |= imgData[i];
		png_data[i * 4 + 3] = 0xff;
	}
	stbi_write_png(out.c_str(), atlasSize, atlasSize, 4, png_data, atlasSize * 4);
}

void FFont::SetCharacterSet(const char* str)
{
	characterSet = str;
}

const char* FFont::GetCharacterSet()
{
	return characterSet.c_str();
}

void FFont::ResetCharacterSet()
{
	characterSet = defaultCharecterSet;
}
