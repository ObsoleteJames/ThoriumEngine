#pragma once

#include "Object/Object.h"
#include "Math/Vectors.h"
#include "Lightmap.generated.h"

STRUCT()
struct FLightmapSettings
{
	GENERATED_BODY()

public:
	PROPERTY(Editable)
	uint lightmapSize = 4096;

	PROPERTY(Editable)
	uint maxLightmapCount = 4;

	PROPERTY(Editable)
	uint directSamples = 64;

	PROPERTY(Editable)
	uint indirectSamples = 256;

	PROPERTY(Editable)
	uint maxBounces = 4;

	PROPERTY(Editable)
	float lightmapDensity = 32.f; // texels per unit

	PROPERTY(Editable, UIType = Color)
	FVector skyColor = FVector(0.f, 0.56f, 1.f);
};
