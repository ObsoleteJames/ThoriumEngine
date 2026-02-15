
#ifndef OBJECT_BUFFER_HLSL
#define OBJECT_BUFFER_HLSL

cbuffer ObjectInfo : register(b3)
{
    float4x4 vSkeletonMatrix[48];
    float4x4 vObjectMatrix;
    float3 vObjectPos;
    float2 vLightmapOffset;
    float2 vLightmapScale;
}

Texture2D vBaseColor : TEXTURE : register(t5);
Texture2D vNormalMap : TEXTURE : register(t6);

SamplerState vBaseColorSampler : SAMPLER : register(s5);
SamplerState vNormalMapSampler : SAMPLER : register(s6);

#endif
