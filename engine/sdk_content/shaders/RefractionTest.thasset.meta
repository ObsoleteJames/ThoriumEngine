
Shader
{
	Name = "RefractionTest";
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
		float4 vScreenPos : TEXCOORD6;
	};

	Property<float> vRefraction(Name = "IOR") = 1.1f;
}

VS
{
	#include "common/vertex.hlsl"

	PS_Input Main(VS_Input input)
	{
		PS_Input output = {
			input.position,
			input.normal,
			input.texCoords,
			input.color,
			input.tangent,
			float3(0, 0, 0),
			float4(0, 0, 0, 0),
			float4(0, 0, 0, 0)
		};

		FinalizeVertex(input, output);
		output.vScreenPos = output.vPositionPs;
		return output;
	}
}

PS
{
	float4 Main(PS_Input input) : SV_TARGET
	{
		float3 refractVec = refract(vCameraDir, -input.vNormalWs, vRefraction);

		float2 uv = input.vScreenPos.xy / input.vScreenPos.w;
		uv = (uv + 1) / 2;
		uv.y = 1 - uv.y;

		uv += refractVec.xy * 0.25f;

		float4 col = SampleFrameBuffer(vFrameBuffer, uv);

		//return float4((refractVec + 1) / 2, 1.f);
		//return float4(uv.x, uv.y, 0.f, 1.f);
		return col;
	}
}
