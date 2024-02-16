
Shader
{
	Name = "Unlit";
	Type = SHADER_FORWARD;
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
		float3 camRelativePos = vCameraPos - input.vPositionWs;
		float shade = max(0.5, dot(normalize(input.vNormalWs), normalize(camRelativePos)));
		float4 col = SampleTexture2D(vBaseColor, input.vTextureCoords) * vColorTint;
		col = mul(shade, col);

		return float4(col.xyz, 1.f);
	}
}
