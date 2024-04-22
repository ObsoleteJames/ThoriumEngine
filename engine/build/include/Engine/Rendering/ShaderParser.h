#pragma once

#include "EngineCore.h"
#include "Shader.h"

struct ENGINE_API FShaderSourceFile
{
	struct FProperty
	{
		FString name;
		FString description;
		FString internalName;
		FString uiGroup;
		SizeType bufferOffset;
		FString initValue;
		int type;
		int uiType;
	};

public:
	FString name;
	FString description;
	FString file;
	int8 type;

	TArray<FProperty> properties;

	SizeType bufferSize;

	FString global;
	//FString vertexShader;
	//FString pixelShader;
	//FString geoShader;

	TArray<TPair<uint8, FString>> shaders;
};

extern bool ENGINE_API ParseShaderSourceFile(const FString& file, FShaderSourceFile& out);
