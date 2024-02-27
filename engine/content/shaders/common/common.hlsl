
#ifndef COMMON_HLSL
#define COMMON_HLSL

#include "common/object_buffer.hlsl"
#include "common/scene_buffer.hlsl"

#define SampleTexture2D(tex, uv) tex.Sample(tex##Sampler, uv)
#define SampleTexture2DLOD(tex, uv, lod) tex.SampleLevel(tex##Sampler, uv, lod)

#define SampleFrameBuffer(fb, uv) fb.Sample(fb##Sampler, uv * vFrameBufferScale)

float3 CalculateNormal(float3 normal, float tangent, float3 biTangent, float2 uv)
{
	float3 normalMap = normalize(SampleTexture2D(vNormalMap, uv).xyz * 2 - 1);
	normalMap.y = -normalMap.y;
	
	normalMap = normalize(lerp(float3(0, 0, 1), normalMap, vNormalIntensity));
	if (vNormalIntensity == 0)
		return normalize(normal);

	float3x3 TBN = {
		tangent.x, biTangent.x, normal.x,
		tangent.y, biTangent.y, normal.y,
		tangent.z, biTangent.z, normal.z
	};
	return normalize(mul(TBN, normalMap));
}

#endif
