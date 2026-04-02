
Shader
{
	Name = "blitFrameBuffer";
    Type = SHADER_INTERNAL;
}

Global
{

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
	Texture2D vInFB : TEXTURE : register(t1);
	SamplerState vSampler : SAMPLER : register(s1);

	cbuffer ScalarData : register(b0)
	{
		float viewportX;
		float viewportY;
		float viewportWidth;
		float viewportHeight;
	}

	float4 Main(PS_Input input) : SV_TARGET
	{
		float2 uv = input.vTextureCoords;

		uv += float2(viewportX, viewportY);
		uv *= float2(viewportWidth, viewportHeight);
		
		return float4(vInFB.Sample(vSampler, uv).xyz, 1.f);
	}
}
