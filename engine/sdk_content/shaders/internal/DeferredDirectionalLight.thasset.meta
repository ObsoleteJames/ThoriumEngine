
Shader
{
	Name = "DeferredDirectionalLight";
	Type = SHADER_INTERNAL;
}

Global
{
	//#include "common/common.hlsl"
	#include "common/scene_buffer.hlsl"
	
	#define SampleTexture2D(tex, uv) tex.Sample(tex##Sampler, uv)
	#define SampleTexture2DLOD(tex, uv, lod) tex.SampleLevel(tex##Sampler, uv, lod)

	struct VS_Input
	{
		#include "common/vertex_input.hlsl"
	};

	struct PS_Input
	{
		#include "common/pixel_input.hlsl"
	};
}

PS
{
	#include "common/shadow_data.hlsl"
	#include "common/lighting.hlsl"
	#include "common/matrix.hlsl"

	//Texture2D vColor : TEXTURE : register(t3);
	Texture2D GBufferA : TEXTURE : register(t4);
	Texture2D GBufferB : TEXTURE : register(t5);
	Texture2D GBufferC : TEXTURE : register(t6);
	Texture2D GBufferD : TEXTURE : register(t7);

	//SamplerState vColorSampler : SAMPLER : register(s3);
	SamplerState GBufferASampler : SAMPLER : register(s4);
	SamplerState GBufferBSampler : SAMPLER : register(s5);
	SamplerState GBufferCSampler : SAMPLER : register(s6);
	SamplerState GBufferDSampler : SAMPLER : register(s7);

	Texture2D vDepth : TEXTURE : register(t8);
	SamplerState vDepthSampler : SAMPLER : register(s8);

	cbuffer LightBuffer : register(b2)
	{
		FDirectionalLight dirLight;
	}

	float3 PositionFromDepth(float depth, float2 uv)
	{
		float z = depth;

		float2 _uv = float2(uv.x, 1 - uv.y);

		float4 clipSpacePos = float4(_uv * 2 - 1, z, 1);
		float4 pos = mul(vInvCameraMatrix, clipSpacePos);
		return pos.xyz / pos.w;
	}

	static const uint sssMaxSteps = 16;
	static const float sssRayMaxDistance = 0.05f;
	static const float sssThickness = 0.02f;
	static const float sssStepLength = sssRayMaxDistance / (float)sssMaxSteps;

	float3 ScreenSpaceShadow(float3 posWs, FDirectionalLight light)
	{
		if (abs(length(posWs - vCameraPos)) > 10)
			return float3(0, 0, 0);

		float3 rayPos = posWs;
		float3 rayDir = light.direction;

		float3 rayStep = rayDir * sssStepLength;

		float occlusion = 0.f;
		float2 rayUv = float2(0.f, 0.f);

		for (uint i = 0; i < sssMaxSteps; i++)
		{
			rayPos += rayStep;
			float4 projectedPos = mul(vCameraMatrix, float4(rayPos, 1.f));
			rayUv = (projectedPos.xy / projectedPos.w) * vFrameBufferScale;
			rayUv = (rayUv + 1) / 2;
			
			if (rayUv.x > 0.f && rayUv.x < 1.f && rayUv.y > 0.f && rayUv.y < 1.f)
			{
				float depth = SampleTexture2DLOD(vDepth, float2(rayUv.x, 1-rayUv.y), 0).r;
				float depthDelta = (projectedPos.z / projectedPos.w) - depth;

				//if (i == sssMaxSteps-1)
				//	return projectedPos.xyz / projectedPos.w;
				if ((depthDelta > 0.0f) && (depthDelta < sssThickness))
				{
					occlusion = 1;
					break;
				}
			}
		}

		return float3(occlusion, 0, 0);
	}

	float4 Main(PS_Input input) : SV_TARGET
	{
		float2 screenUV = input.vTextureCoords * vFrameBufferScale;

		float depth = SampleTexture2D(vDepth, screenUV).r;
		if (depth == 1.f)
			discard;

		float3 worldPos = PositionFromDepth(depth, input.vTextureCoords);

		//return float4(worldPos, 1);

		float3 normal = normalize(SampleTexture2D(GBufferA, screenUV).xyz * 2.f - 1.f);
		float3 diffuse = SampleTexture2D(GBufferC, screenUV).xyz;
		float4 material = SampleTexture2D(GBufferB, screenUV);
		float metallic = material.r;
		float roughness = material.g;
		float ao = material.b;

		float shadow = DirectionalLightShadow(dirLight, worldPos, 8);

		if (shadow == 1.f)
			discard;

		//float3 sss = ScreenSpaceShadow(worldPos, dirLight);
		//if (sss.r > 0.f)
		//	shadow = sss;

		float3 V = normalize(vCameraPos - worldPos);
		float3 H = normalize(V + dirLight.direction);
		float3 radiance = dirLight.color * dirLight.intensity;

		float3 F0 = float3(0.04, 0.04, 0.04);
		F0 = lerp(F0, diffuse, metallic);
		float3 F = fresnelSchlick(max(dot(H, V), 0), F0);

		float NDF = DistributionGGX(normal, H, roughness);
		float G = GeometrySmith(normal, V, dirLight.direction, roughness);

		float3 numerator = mul(NDF * G, F);
		float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, dirLight.direction), 0.0)  + 0.0001;
		float specular = numerator / denominator;

		float3 kS = F;
		float3 kD = float3(1, 1, 1) - kS;
		kD = mul((1.0 - metallic), kD);
		float NdotL = max(dot(normal, dirLight.direction), 0);

		float3 color = (kD * diffuse / PI + specular) * radiance * NdotL;

		return float4(color * (1-shadow), 1.f);
	}
}
