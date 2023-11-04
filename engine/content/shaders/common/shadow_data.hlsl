
#ifndef SHADOW_DATA_HLSL
#define SHADOW_DATA_HLSL

cbuffer ShadowData : register(b4)
{
	float4x4 vSpotShadowMatrix[8];
	float4x4 vSunShadowMatrix[4];
	float4 vPointShadowPos[8];

	// -1 if there is none
	int4 vSpotShadowId[2];
	int4 vPointShadowId[2];

	float4 vSpotShadowBias[2];
	float4 vPointShadowBias[2];
	
	int vSunShadowId;
	float vSunShadowBias;
}

Texture2DArray vSpotShadows : TEXTURE : register(t0);
TextureCubeArray vPointShadows : TEXTURE : register(t1);
Texture2D vSunShadow : TEXTURE : register(t2);

SamplerState vSpotShadowsSampler : SAMPLER : register(s0);
SamplerState vPointShadowsSampler : SAMPLER : register(s1);
SamplerState vSunShadowSampler : SAMPLER : register(s2);

#endif
