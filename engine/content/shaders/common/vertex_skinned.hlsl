
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
    float4x4 skeletonTransform = float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); \
    if (any(input.boneWeight)) {\
        if (input.boneIndices[0] != -1) \
            skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[0]], input.boneWeight.x); \
        if (input.boneIndices[1] != -1) \
            skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[1]], input.boneWeight.y); \
        if (input.boneIndices[2] != -1) \
            skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[2]], input.boneWeight.z); \
        if (input.boneIndices[3] != -1) \
            skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[3]], input.boneWeight.w); \
        \
        outPos = mul(skeletonTransform, outPos); \
    } \
    outPos = mul(vObjectMatrix, outPos); \
    \
    float3x3 normalMat = any(input.boneWeight) ? transpose((float3x3)inverse(mul(skeletonTransform, vObjectMatrix))) : transpose((float3x3)inverse(vObjectMatrix));\
    output.vNormalWs = normalize(mul(normalMat, output.vNormalWs));\
    output.vTangentUWs = normalize(mul(normalMat, output.vTangentUWs));\
    output.vTangentVWs = normalize(cross(output.vTangentUWs, output.vNormalWs));\
    \
    output.vPositionWs = (float3)outPos; \
    output.vPositionPs = mul(vCameraMatrix, outPos); \
}

#endif
