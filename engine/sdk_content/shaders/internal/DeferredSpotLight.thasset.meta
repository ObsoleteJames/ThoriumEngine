
Shader
{
	Name = "DeferredSpotLight";
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

	Property<int> vTestProperty(Name = "Test Property");
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
		return float4(0.5f, 0.5f, 1.f, 1.f);
	}
}
