
#ifndef LIGHTING_HLSL
#define LIGHTING_HLSL

#include "common/light_structs.hlsl"

static const float PI = 3.14159265359;

static const int kPoissonSampleCount = 16;
static const float2 kPoissonDisk16[kPoissonSampleCount] =
{
	float2(-0.94201624f, -0.39906216f),
	float2(0.94558609f, -0.76890725f),
	float2(-0.09418410f, -0.92938870f),
	float2(0.34495938f, 0.29387760f),
	float2(-0.91588581f, 0.45771432f),
	float2(-0.81544232f, -0.87912464f),
	float2(-0.38277543f, 0.27676845f),
	float2(0.97484398f, 0.75648379f),
	float2(0.44323325f, -0.97511554f),
	float2(0.53742981f, -0.47373420f),
	float2(-0.26496911f, -0.41893023f),
	float2(0.79197514f, 0.19090188f),
	float2(-0.24188840f, 0.99706507f),
	float2(-0.81409955f, 0.91437590f),
	float2(0.19984126f, 0.78641367f),
	float2(0.14383161f, -0.14100790f)
};

float Hash12(float2 p)
{
	float3 p3 = frac(float3(p.xyx) * 0.1031f);
	p3 += dot(p3, p3.yzx + 33.33f);
	return frac((p3.x + p3.y) * p3.z);
}

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

					int pcfSamples = clamp(sampleCount, 1, kPoissonSampleCount);
					float2 texelSize = float2(1.0f / texWidth, 1.0f / texHeight);
					float2 mapUvBase = float2(projCoords.x / 4 + (0.25f * i), 1.f - projCoords.y);
					float cascadeMinX = 0.25f * i;
					float cascadeMaxX = cascadeMinX + 0.25f;
					float clampPadding = 2.0f * texelSize.x;
					float filterRadius = (i == 0 ? 2.0f : 1.5f);
					float bias = (i == 0 ? vSunShadowBias : 0.0005f);

					float angle = Hash12(posWs.xz + float2(i * 17.31f, i * 3.17f)) * 6.2831853f;
					float2 rot = float2(cos(angle), sin(angle));

					for (int j = 0; j < pcfSamples; j++)
					{
						float2 disk = kPoissonDisk16[j];
						float2 rotated = float2(
							disk.x * rot.x - disk.y * rot.y,
							disk.x * rot.y + disk.y * rot.x
						);
						float2 offset = rotated * texelSize * filterRadius;

						float2 mapUv = mapUvBase + offset;
						mapUv.x = clamp(mapUv.x, cascadeMinX + clampPadding, cascadeMaxX - clampPadding);
						mapUv.y = clamp(mapUv.y, clampPadding, 1.0f - clampPadding);

						float shadowDepth = SampleTexture2DLOD(vSunShadow, mapUv, 0).r;
						if (fragPosLightSpace.z - bias > shadowDepth)
							averageShadow = averageShadow + 1.f;
					}

					shadow = averageShadow / pcfSamples;
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
