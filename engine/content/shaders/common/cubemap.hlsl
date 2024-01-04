
#ifndef CUBEMAP_HLSL
#define CUBEMAP_HLSL

struct FCubeMap
{
	float4 rotation; // Quaternion
	float3 position;
	float3 size;
	bool bIBL;
};

cbuffer CubeMapBuffer : register(b5)
{
	FCubeMap vCubeMapData;
}

TextureCubeArray vCubeMap : TEXTURE : register(t3);
SamplerState vCubeMapSampler : SAMPLER : register(s3);

#endif
