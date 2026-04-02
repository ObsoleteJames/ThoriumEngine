
#ifndef VERTEX_HLSL
#define VERTEX_HLSL

#include "common/matrix.hlsl"

#define ProcessVertex(input) \
{ \
	input.position, \
	input.normal, \
    input.texCoords, \
	input.color, \
    input.tangent, \
	float3(0.f, 0.f, 0.f), \
	float4(0.f, 0.f, 0.f, 0.f) \
}

#define ProcessVertexB(input) \
	input.position, \
	input.normal, \
    input.texCoords, \
	input.color, \
    input.tangent, \
	float3(0.f, 0.f, 0.f), \
	float4(0.f, 0.f, 0.f, 0.f)


#define FinalizeVertex( input, output ) \
{ \
    float4 outPos = float4(output.vPositionWs, 1.f); \
    \
    outPos = mul(vObjectMatrix, outPos); \
    \
    float3x3 normalMat = transpose((float3x3)inverse(vObjectMatrix));\
    output.vNormalWs = normalize(mul(normalMat, output.vNormalWs));\
    output.vTangentUWs = normalize(mul(normalMat, output.vTangentUWs));\
    output.vTangentVWs = normalize(cross(output.vTangentUWs, output.vNormalWs));\
    \
    output.vPositionWs = (float3)outPos; \
    output.vPositionPs = mul(vCameraMatrix, outPos); \
}

#endif
