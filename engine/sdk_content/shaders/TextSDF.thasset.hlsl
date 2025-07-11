
Shader
{
	Name = "TextSDF";
	Type = SHADER_INTERNAl;
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

	cbuffer TextSDFBuffer : register(b4)
	{
		float4 color;
		float4 outlineColor;

		float thickness;
		float outline;

		unsigned int uvStride;
		unsigned int uvIndex;
	}
}

VS
{
	#include "common/vertex.hlsl"

	PS_Input Main(VS_Input input)
	{
		PS_Input output = ProcessVertex(input);

		FinalizeVertex(input, output);
		return output;
	}
}

PS
{
	float4 Main(PS_Input input) : SV_TARGET
	{
		float uvX = uvIndex % uvStride;
		float uvY = uvIndex / uvStride;

		float2 uv = input.vTextureCoords + float2(uvX, uvY);
		uv = uv / uvStride;

		float sdf = SampleTexture2D(vBaseColor, uv).r;
		if (sdf < thickness)
			return float4(color.xyz, 1.f);
		if (outline != 0.f && sdf < outline)
			return float4(outlineColor.xyz, 1.f);

		discard;
		return float4(0.f, 0.f, 0.f, 1.f);
	}
}
