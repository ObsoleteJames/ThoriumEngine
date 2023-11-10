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
	WString file;
	int8 type;

	TArray<FProperty> properties;

	SizeType bufferSize;

	FString global;
	FString vertexShader;
	FString pixelShader;
	FString geoShader;
};

extern bool ENGINE_API ParseShaderSourceFile(const WString& file, FShaderSourceFile& out);
