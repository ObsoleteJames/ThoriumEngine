
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


#define FinalizeVertex( input, output ) \
{ \
    float4 outPos = float4(output.vPositionWs, 1.f); \
    \
    float4x4 skeletonTransform = float4x4(1.f, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1); \
    skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[0]], input.boneWeight.x); \
    skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[1]], input.boneWeight.y); \
    skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[2]], input.boneWeight.z); \
    skeletonTransform += mul(vSkeletonMatrix[input.boneIndices[3]], input.boneWeight.w); \
    \
    outPos = mul(skeletonTransform, outPos); \
    outPos = mul(vObjectMatrix, outPos); \
    \
    float3x3 normalMat = transpose((float3x3)inverse(mul(skeletonTransform, vObjectMatrix)));\
    output.vNormalWs = mul(normalMat, output.vNormalWs);\
    output.vTangentUWs = mul(normalMat, output.vTangentUWs);\
    output.vTangentVWs = normalize(cross(output.vNormalWs, output.vTangentUWs));\
    \
    output.vPositionWs = (float3)outPos; \
    output.vPositionPs = mul(vCameraMatrix, outPos); \
}

#endif
