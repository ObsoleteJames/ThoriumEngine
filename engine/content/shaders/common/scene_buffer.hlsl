
#ifndef SCENE_BUFFER_HLSL
#define SCENE_BUFFER_HLSL

cbuffer SceneInfo : register(b1)
{
    float4x4 vCameraMatrix;
    float4x4 vCameraView;
    float4x4 vCameraProjection;
    
    float3 vCameraPos;
    float padding_vCameraPos;
    float3 vCameraDir;
    float padding_vCameraDir;
    
    float vCurTime;
}

#endif
