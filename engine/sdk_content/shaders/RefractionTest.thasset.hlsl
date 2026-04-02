
Shader
{
	Name = "RefractionTest";
	Type = SHADER_FORWARD;
}

Global
{
	#include "common/common.hlsl"

	struct PS_Input
	{
		#include "common/pixel_input.hlsl"
		float4 vScreenPos : TEXCOORD6;
	};

	Property<float> vRefraction(Name = "IOR") = 1.1f;
	Property<float> vThickness(Name = "Thickness") = 0.15f;
	Property<float> vEdgeFade(Name = "Edge Fade") = 12.0f;
}

VS
{
	#include "common/vertex.hlsl"

	struct VS_Input
	{
		#include "common/vertex_input.hlsl"
	};

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

VS_SKINNED
{
	#include "common/vertex_skinned.hlsl"
	
	struct VS_Input
	{
		#include "common/vertex_input_skinned.hlsl"
	};

    PS_Input Main(VS_Input input)
    {
		//PS_Input output = ProcessVertex(input);
		PS_Input output = {
			input.position,
			input.normal,
    		input.texCoords,
			input.color,
    		input.tangent,
			float3(0.f, 0.f, 0.f),
			float4(0.f, 0.f, 0.f, 0.f),
			float4(0.f, 0.f, 0.f, 0.f)
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
		float3 normalWs = normalize(input.vNormalWs);
		float3 viewDir = normalize(vCameraPos - input.vPositionWs);
		float eta = 1.0f / max(0.001f, vRefraction);

		float3 refractDir = refract(-viewDir, normalWs, eta);
		if (dot(refractDir, refractDir) < 0.001f)
		{
			refractDir = reflect(-viewDir, normalWs);
		}

		float2 baseUv = input.vScreenPos.xy / input.vScreenPos.w;
		baseUv = baseUv * 0.5f + 0.5f;
		baseUv.y = 1 - baseUv.y;

		float3 refractPosWs = input.vPositionWs + refractDir * 0.25f;
		float4 refractPosPs = mul(vCameraMatrix, float4(refractPosWs, 1.0f));
		float2 refractUv = refractPosPs.xy / max(0.0001f, refractPosPs.w);
		refractUv = refractUv * 0.5f + 0.5f;
		refractUv.y = 1 - refractUv.y;

		float2 edgeDist = min(refractUv, 1.0f - refractUv);
		float edgeMask = saturate(min(edgeDist.x, edgeDist.y) * vEdgeFade);

		float4 refractCol = SampleFrameBuffer(vFrameBuffer, refractUv);
		//refractCol.r = 1 - refractCol.r;
		float4 baseCol = SampleFrameBuffer(vFrameBuffer, baseUv);
		//float4 col = lerp(baseCol, refractCol, edgeMask);

		//return float4((refractVec + 1) / 2, 1.f);
		//return float4(uv.x, uv.y, 0.f, 1.f);
		return refractCol;
	}
}
