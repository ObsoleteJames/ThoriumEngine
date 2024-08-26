
Shader
{
	Name = "Billboard";
	Type = SHADER_DEBUG;
}

Global
{
	#include "common/common.hlsl"

	struct PS_Input
	{
		#include "common/pixel_input.hlsl"
	};
}

VS
{
	#include "common/vertex.hlsl"

	static const float3 verts[] = {
		float3(-0.5f, -0.5f, 0),
		float3(-0.5f,  0.5f, 0),
		float3( 0.5f, -0.5f, 0),
		float3( 0.5f, -0.5f, 0),
		float3(-0.5f,  0.5f, 0),
		float3( 0.5f,  0.5f, 0)
	};

	PS_Input Main(uint id : SV_VERTEXID)
	{
		PS_Input output;

		float4 outPos = float4(verts[id], 1.f);
		float3x3 camRot = transpose((float3x3)vCameraView);

		outPos = float4(mul(camRot, outPos.xyz), 1.f);
		outPos = mul(vObjectMatrix, outPos);

		output.vPositionWs = (float3)outPos;
		output.vNormalWs = float3(0, 0, 1);
		output.vTextureCoords = verts[id].xy;
		output.vTextureCoords.x = output.vTextureCoords.x - 0.5f;
		output.vTextureCoords.y = -output.vTextureCoords.y - 0.5f;
		output.vVertexColor = float3(0, 0, 0);
		output.vTangentUWs = float3(0, 0, 0);
		output.vTangentVWs = float3(0, 0, 0);
		output.vPositionPs = mul(vCameraMatrix, outPos);
		//output.vPositionPs = float4(outPos, 1.f);
		return output;
	}
}

PS
{
	float4 Main(PS_Input input) : SV_TARGET
	{
		float4 tex = SampleTexture2D(vBaseColor, input.vTextureCoords);
		if (tex.a == 0.f)
			discard;

		return tex;
	}
}
