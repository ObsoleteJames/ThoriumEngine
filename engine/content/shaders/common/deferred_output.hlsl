
#ifndef DEFERRED_OUTPUT_HLSL
#define DEFERRED_OUTPUT_HLSL

struct PS_Output
{
	float4 vColor : SV_TARGET0;
	float4 vNormal : SV_TARGET1;
	float4 vMaterial : SV_TARGET2;
	float4 vDiffuse : SV_TARGET3;
	float4 GBufferD : SV_TARGET4;
};

#endif
