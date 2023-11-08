#pragma once

#include "Object/Object.h"
#include "Math/Math.h"
#include "Math/Vectors.h"
#include "HitInfo.generated.h"

STRUCT()
struct FHitInfo
{
	GENERATED_BODY()

public:
	PROPERTY()
	TObjectPtr<CObject> hitObj;

	PROPERTY()
	FVector position;

	PROPERTY()
	FVector normal;

	PROPERTY()
	float distance;

};
