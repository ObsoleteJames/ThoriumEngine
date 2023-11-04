
#ifndef LIGHT_STRUCTS_HLSL
#define LIGHT_STRUCTS_HLSL

struct FDirectionalLight
{
	float3 direction;
	float _padding1;
	float3 color;
	float intensity;
	float3 _padding2;
	int shadowIndex;
};

struct FPointLight
{
	float3 position;
	float _padding1;
	float3 color;
	float intensity;
	float2 _padding2;
	float range;
	int shadowIndex;
};

struct FSpotLight
{
	float3 position;
	float _padding1;
	float3 direction;
	float _padding2;
	float3 color;
	float intensity;
	float innerConeAngle;
	float outerConeAngle;
	float range;
	int shadowIndex;
};

#endif
