
Shader
{
	Name = "PPExposure";
	Type = SHADER_INTERNAL;
}

Global
{
	#include "common/common.hlsl"

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
	Texture2D vColor : TEXTURE : register(t0);
	SamplerState vColorSampler : SAMPLER : register(s0);

	float4 Main(PS_Input input) : SV_TARGET
	{
		float3 color = SampleFrameBuffer(vColor, input.vTextureCoords).xyz * vExposure;
		//color = pow(abs(color), abs(float3(1 / vGamma, 1 / vGamma, 1 / vGamma)));

		return float4(color, 1.f);
	}
}
