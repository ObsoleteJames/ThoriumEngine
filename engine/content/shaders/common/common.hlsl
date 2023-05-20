
#ifndef COMMON_HLSL
#define COMMON_HLSL

#include "common/object_buffer.hlsl"
#include "common/scene_buffer.hlsl"

#define SampleTexture2D(tex, uv) tex.Sample(tex##Sampler, uv)
#define SampleTexture2DLOD(tex, uv, lod) tex.SampleLevel(tex##Sampler, uv, lod)

#endif
