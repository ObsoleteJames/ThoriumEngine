
#ifndef SCENE_BUFFER_HLSL
#define SCENE_BUFFER_HLSL

cbuffer SceneInfo : register(b1)
{
    float4x4 vCameraMatrix;
    float4x4 vCameraView;
    float4x4 vCameraProjection;
    float4x4 vInvCameraMatrix;
    float4x4 vInvCameraView;
    float4x4 vInvCameraProjection;

    float3 vCameraPos;
    float padding_vCameraPos;
    float3 vCameraDir;
    float padding_vCameraDir;
    
    float vCurTime;

    float vExposure;
    float vGamma;

    // The GBuffers can be a larger size than the viewport,
    // so when sampling a framebuffer we need to account for the size difference using this value.
    float2 vFrameBufferScale; 
}

// either the last frame (for forward and deferred pass), or the last pass (for when drawing transparent materials).
Texture2D vFrameBuffer : TEXTURE : register(t3);
SamplerState vFrameBufferSampler : SAMPLER : register(s3);

#endif
