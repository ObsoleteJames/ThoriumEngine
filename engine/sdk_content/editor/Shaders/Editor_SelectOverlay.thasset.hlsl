
Shader
{
	Name = "Editor_SelectOverlay";
	Type = SHADER_INTERNAL;
}

Global
{
	#include "common/common.hlsl"

	struct PS_Input
	{
		#include "common/pixel_input.hlsl"
		float4 vScreenPos : TEXCOORD6;
	};
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
	static const int bayer2[2 * 2] = {
		0, 2,
		3, 1
	};

	static const int bayer4[4 * 4] = {
		0, 8, 2, 10,
		12, 4, 14, 6,
		3, 11, 1, 9,
		15, 7, 13, 5
	};

	static const int bayer8[8 * 8] = {
		0, 32, 8, 40, 2, 34, 10, 42,
		48, 16, 56, 24, 50, 18, 58, 26,
		12, 44, 4, 36, 14, 46, 6, 38,
		60, 28, 52, 20, 62, 30, 54, 22,
		3, 35, 11, 43, 1, 33, 9, 41,
		51, 19, 59, 27, 49, 17, 57, 25,
		15, 47, 7, 39, 13, 45, 5, 37,
		63, 31, 55, 23, 61, 29, 53, 21
	};

	float GetBayer2(int x, int y)
	{
		return float(bayer2[(x % 2) + (y % 2) * 2]) * (1.0f / 4.0f) - 0.5f;
	}

	float GetBayer4(int x, int y)
	{
		return float(bayer4[(x % 4) + (y % 4) * 4]) * (1.0f / 16.0f) - 0.5f;
	}

	float GetBayer8(int x, int y)
	{
		int index = (x % 8) + ((y % 8) * 8);

		return float(bayer8[index]) * (1.0f / 64.0f) - 0.5f;
	}

	Texture2D depthTex : TEXTURE : register(t2);
	SamplerState depthTexSampler : SAMPLER : register(s2);

	float4 Main(PS_Input i) : SV_TARGET
	{
		float2 uv = i.vScreenPos.xy / i.vScreenPos.w;
		uv = (uv + 1) / 2;
		uv.y = 1 - uv.y;

		int x = uv.x * float(vViewport.x);
		int y = uv.y * float(vViewport.y);

		float curDepth = i.vScreenPos.z / i.vScreenPos.w;
		float depth = SampleTexture2D(depthTex, uv * vFrameBufferScale).r;

		float cutOff = 0.5f;
		if (curDepth - 0.0001 > depth)
		 	cutOff = 0.4f;

		float alpha = cutOff + 0.5f * GetBayer8(x, y);

		if (alpha < 0.5f)
			discard;

		float3 col = float3(1.f, 0.54f, 0.f);
		float3 col2 = float3(1.f, 0.4f, 0.f);

		//return float4(depth, depth, depth, 1.f);
		// if (curDepth - 0.0001 > depth)
		// 	return float4(col2, 1.f);

		return float4(col, 1.f);
	}
}
