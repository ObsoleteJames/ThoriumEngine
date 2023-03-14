
#ifndef FORWARD_LIGHT_DATA_HLSL
#define FORWARD_LIGHT_DATA_HLSL

#include "common/light_structs.hlsl"

cbuffer SceneLights : register(b2)
{
    FDirectionalLight directionalLights[2];
	FPointLight pointLights[8];
	FSpotLight spotLights[8];

	int numDirectionalLights;
	int numPointLights;
	int numSpotLights;
}

#endif
