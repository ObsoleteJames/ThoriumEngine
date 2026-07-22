#pragma once

#include "EngineCore.h"
#include "Math/Vectors.h"
#include "Assets/Mesh.h"

class CTexture;

struct ENGINE_API FGlyph
{
	int width;
	int height;
	int bearingX;
	int bearingY;
	uint advance;

	// Texture coordinate data
	FVector2 uvOffset;
	FVector2 uvScale;
};

struct FFontSettings
{
	int fontSize = 12;
	bool bAntialiasing = true;
	bool bSDF = false; // wether to render in SDF mode
	int SDFspread = 4;
};

class ENGINE_API FFont
{
public:
	FFont(const FString& file, const FFontSettings& settings);
	~FFont();

	inline const uint8* GetAtlasData() const { return imgData; }
	inline uint64 GetAtlasSize() const { return atlasSize; }

	inline CTexture* GetAtlasTexture() const { return fontTexture; }

	void GenerateMesh(TArray<FVertex>& outVerts, const char* text, const FVector2& bounds, int lineSpacing, int characterSpacing);
	inline void GenerateMesh(TArray<FVertex>& outVerts, const char* text) { GenerateMesh(outVerts, text, FVector2(), 4, 0); }
	inline void GenerateMesh(TArray<FVertex>& outVerts, const char* text, int lineSpacing, int characterSpacing) { GenerateMesh(outVerts, text, FVector2(), lineSpacing, characterSpacing); }

	FVector2 CalculateTextSize(const char* text) const;

	void SavePng(const FString& out);

public:
	static void SetCharacterSet(const char*);
	static const char* GetCharacterSet();
	static void ResetCharacterSet();

private:
	TMap<uint32, FGlyph> glyphs;

	TObjectPtr<CTexture> fontTexture = nullptr;
	uint8* imgData = nullptr;
	uint64 atlasSize = 0;
	
	float glyphHeight;

	FString file;
	FFontSettings settings;
};
