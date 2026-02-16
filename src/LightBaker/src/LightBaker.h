#pragma once

#include "Util/Core.h"
#include "Util/Guid.h"
#include "Math/Vectors.h"
#include "Rendering/Lightmap.h"

class CWorld;

struct FLightmapAtlas
{
	uint32 width;
	uint32 height;
	TArray<FVector> data;
};

//struct FBakeSettings
//{
//	uint lightmapSize = 4096;
//	uint maxLightmapCount = 4;
//	uint directSamples = 64;
//	uint indirectSamples = 256;
//	uint maxBounces = 4;
//	float lightmapDensity = 32.f; // texels per unit
//	FVector skyColor = FVector(0.f, 0.56f, 1.f);
//};

struct FLightmapMeshRef
{
	FGuid entityId;
	FGuid componentId;
	uint32 meshIndex = 0;
	int32 lightmapId = -1;
	FVector2 lightmapPos = FVector2(0.0f);
	FVector2 lightmapScale = FVector2(1.0f);
};

struct FBakeResult
{
	bool bSuccess = false;
	TArray<FLightmapAtlas> lightmaps;
	TArray<FLightmapMeshRef> meshRefs;
};

class CLightBaker
{
public:
	CLightBaker(CWorld* world);

	void BakeAll(const FLightmapSettings& settings = FLightmapSettings());

	void BakeLightmaps();
	void BakeLightProbes();

	void SaveData();

	inline const FBakeResult& GetResult() const { return result; }

private:
	void CreateAtlas();

private:
	CWorld* world;

	FBakeResult result;
	FLightmapSettings settings;
};
