#pragma once

#include "EngineCore.h"
#include "Math/Vectors.h"

struct FShaderCameraData
{
	FMatrix view;
	FMatrix projection;
	FVector position;
	int _padding0;
};

struct FLightData
{
	FVector color;
	int _padding0;

	FVector direction;
	int _padding1;

	FVector position;
	int _padding2;

	float lightUV[2];
	float attenuation;
	float texSize;
};

struct FShaderLightData
{
	FLightData lights[64];
	int numLights;
};
