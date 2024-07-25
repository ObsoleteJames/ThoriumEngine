
Shader
{
	Name = "DebugNormalForward";
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
	#include "common/normal_map.hlsl"

	float4 Main(PS_Input input) : SV_TARGET
	{
		float3 N = CalculateNormal(input.vNormalWs, input.vTangentUWs, input.vTangentVWs, input.vTextureCoords);
		
		return float4((N + 1.f) / 2.f, 1.f);
	}
}
