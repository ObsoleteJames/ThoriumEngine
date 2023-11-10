#pragma once

#include "Math/Vectors.h"

struct FBone
{
	FString name;
	uint32 parent;
	FVector position;
	FVector direction;
	float roll;
};

struct FSkeleton
{
	TArray<FBone> bones;
};
