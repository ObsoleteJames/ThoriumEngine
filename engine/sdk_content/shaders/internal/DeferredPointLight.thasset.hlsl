
Shader
{
	Name = "DeferredPointLight";
	Type = SHADER_INTERNAL;
}

Global
{
	#include "common/scene_buffer.hlsl"
	
	cbuffer ObjectInfo : register(b3)
	{
		float4x4 vSkeletonMatrix[48];
		float4x4 vObjectMatrix;
		float3 vObjectPos;
	}

	#define SampleTexture2D(tex, uv) tex.Sample(tex##Sampler, uv)
	#define SampleTexture2DLOD(tex, uv, lod) tex.SampleLevel(tex##Sampler, uv, lod)

	struct VS_Input
	{
		#include "common/vertex_input.hlsl"
	};

	struct PS_Input
	{
		float4 vPositionPs : SV_POSITION;
		float3 vPositionWs : TEXCOORD0;
		float4 vScreenPos : TEXCOORD1;
	};
}

VS
{
	#include "common/vertex.hlsl"

	PS_Input Main(VS_Input input)
	{
		PS_Input output = { float4(0, 0, 0, 0),  input.position, float4(0, 0, 0, 0) };

		float4 outPos = float4(output.vPositionWs, 1.f);
		outPos = mul(vObjectMatrix, outPos);
		output.vPositionWs = (float3)outPos;
		output.vPositionPs = mul(vCameraMatrix, outPos);
		output.vScreenPos = output.vPositionPs;
		//output.vTextureCoords = ((output.vPositionPs.xy / output.vPositionPs.w) + 1) / 2;
		return output;
	}
}

PS
{
	#include "common/shadow_data.hlsl"
	#include "common/lighting.hlsl"
	#include "common/matrix.hlsl"

	Texture2D GBufferA : TEXTURE : register(t4);
	Texture2D GBufferB : TEXTURE : register(t5);
	Texture2D GBufferC : TEXTURE : register(t6);
	Texture2D GBufferD : TEXTURE : register(t7);

	SamplerState GBufferASampler : SAMPLER : register(s4);
	SamplerState GBufferBSampler : SAMPLER : register(s5);
	SamplerState GBufferCSampler : SAMPLER : register(s6);
	SamplerState GBufferDSampler : SAMPLER : register(s7);

	Texture2D vDepth : TEXTURE : register(t8);
	SamplerState vDepthSampler : SAMPLER : register(s8);

	cbuffer LightBuffer : register(b2)
	{
		FPointLight light;
	}

	float3 PositionFromDepth(float depth, float2 uv)
	{
		float z = depth;

		float2 _uv = float2(uv.x, 1 - uv.y);

		float4 clipSpacePos = float4(_uv * 2 - 1, z, 1);
		float4 pos = mul(vInvCameraMatrix, clipSpacePos);
		return pos.xyz / pos.w;
	}

	float4 Main(PS_Input input) : SV_TARGET
	{
		float2 screenUV = (((input.vScreenPos.xy / input.vScreenPos.w) + 1) / 2);
		screenUV.y = 1 - screenUV.y;

		float depth = SampleTexture2D(vDepth, screenUV * vFrameBufferScale).r;
		if (depth == 1.f)
			discard;

		float3 worldPos = PositionFromDepth(depth, screenUV);

		screenUV = screenUV * vFrameBufferScale;

		float3 normal = normalize(SampleTexture2D(GBufferA, screenUV).xyz * 2.f - 1.f);
		float3 diffuse = SampleTexture2D(GBufferC, screenUV).xyz;
		float4 material = SampleTexture2D(GBufferB, screenUV);
		float metallic = material.r;
		float roughness = material.g;
		float ao = material.b;

		float3 V = normalize(vCameraPos - worldPos);
		float3 L = normalize(light.position - worldPos);
		float3 H = normalize(V + L);

		float distance = length(light.position - worldPos);
		float attenuation = 1 - saturate(distance / light.range);
		float3 radiance = light.color * attenuation * light.intensity;

		if (attenuation < 0.001f)
			discard;

		float3 F0 = float3(0.04, 0.04, 0.04);
		F0 = lerp(F0, diffuse, metallic);
		float3 F = fresnelSchlick(max(dot(H, V), 0), F0);

		float NDF = DistributionGGX(normal, H, roughness);
		float G = GeometrySmith(normal, V, L, roughness);

		float3 numerator = mul(NDF * G, F);
		float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0)  + 0.0001;
		float3 specular = numerator / denominator;

		float3 kS = F;
		float3 kD = float3(1, 1, 1) - kS;
		kD = mul((1.0 - metallic), kD);

		float NdotL = max(dot(normal, L), 0);

		float3 color = (kD * diffuse / PI + specular) * radiance * NdotL;

		return float4(color, 1.f);
	}
}
