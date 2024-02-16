#pragma once

#include "Math/Vectors.h"
#include "Math/Transform.h"

struct FBone
{
	FString name;
	uint32 parent;
	FVector position;
	FQuaternion rotation;
};

struct FSkeleton
{
	// inverse model matrices of all bones
	TArray<FMatrix> invModel; 

	TArray<FBone> bones;
};

// helper class for transforming skeletons
struct FSkeletonInstance
{
	TArray<FTransform> bones;
};
