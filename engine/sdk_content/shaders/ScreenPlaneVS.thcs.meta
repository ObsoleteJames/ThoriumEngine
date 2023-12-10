
Shader
{
	Name = "ScreenPlaneVS";
	Type = SHADER_INTERNAL;
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
		float3(-1, 1, 0),
		float3(3, 1, 0),
		float3(-1, -3, 0)
	};

	static const float2 uv[] = {
		float2(0, 0),
		float2(2, 0),
		float2(0, 2)
	};

	PS_Input Main(uint id : SV_VERTEXID)
	{
		PS_Input output;

		output.vPositionWs = verts[id];
		output.vNormalWs = float3(0, 0, 1);
		output.vTextureCoords = uv[id].xy;
		output.vVertexColor = float3(0, 0, 0);
		output.vTangentUWs = float3(0, 0, 0);
		output.vTangentVWs = float3(0, 0, 0);
		output.vPositionPs = float4(verts[id], 1);
		return output;
	}
}
