
#ifndef LIGHT_STRUCTS_HLSL
#define LIGHT_STRUCTS_HLSL

struct FDirectionalLight
{
	float3 direction;
	float _padding1;
	float3 color;
	float intensity;
};

struct FPointLight
{
	float3 position;
	float _padding1;
	float3 color;
	float intensity;
	float range;
	float3 _padding2;
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
	float _padding3;
};

#endif
