
#ifndef LIGHTING_HLSL
#define LIGHTING_HLSL

#include "common/light_structs.hlsl"

static const float PI = 3.14159265359;

float DirectionalLightShadow(FDirectionalLight light, float3 posWs, int sampleCount)
{
	float shadow = 0.f;
	if (light.shadowIndex != -1)
	{
		for (int i = 0; i < 4; i++)
		{
			float4x4 shadowMat = vSunShadowMatrix[0];
			switch (i)
			{
			case 1:
				shadowMat = vSunShadowMatrix[1];
				break;
			case 2:
				shadowMat = vSunShadowMatrix[2];
				break;
			case 3:
				shadowMat = vSunShadowMatrix[3];
				break;
			}

			float4 fragPosLightSpace = mul(shadowMat, float4(posWs, 1.f));
			float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
			projCoords = projCoords * 0.5 + 0.5;

			if (projCoords.x < 1 && projCoords.y < 1 && projCoords.x > 0 && projCoords.y > 0)
			{
				if (i > 1)
				{
					float shadowDepth = SampleTexture2DLOD(vSunShadow, float2(projCoords.x / 4 + (0.25f * i), 1.f - projCoords.y), 0).r;
					if (fragPosLightSpace.z - 0.0002 > shadowDepth)
						return 1.f;
				}
				else
				{
					float averageShadow = 0.f;

					uint texWidth;
					uint texHeight;
					uint texLevels;
					vSunShadow.GetDimensions(0, texWidth, texHeight, texLevels);

					int sampleSqr = sampleCount * sampleCount;

					for (int j = 0; j < sampleSqr; j++)
					{
						float pixelSize = 1.f / texHeight;
						float2 offset = float2((float(j % sampleCount) - 0.5f), ((float(j) / sampleCount) - 0.5f)) * pixelSize;
						float2 mapUv = float2(projCoords.x / 4 + (0.25f * i), 1.f - projCoords.y);

						mapUv = mapUv + offset;
						mapUv.x = clamp(mapUv.x, 0.25f * i, (0.25f * i) + 0.248888f);
						mapUv.y = clamp(mapUv.y, 0, 1);

						float shadowDepth = SampleTexture2DLOD(vSunShadow, mapUv, 0).r;

						if (fragPosLightSpace.z - (i == 0 ? vSunShadowBias : 0.0005) > shadowDepth)
							averageShadow = averageShadow + 1.f;
					}

					shadow = averageShadow / sampleSqr;
					break;
				}
			}
		}
	}
	return shadow;
}

float PointLightShadow(FPointLight light, float3 posWs, int sampleCount)
{
	return 0.f;
}

float SpotLightShadow(FSpotLight light, float3 posWs, int sampleCount)
{
	return 0.f;
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + mul(pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0), max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0);
}

#endif
