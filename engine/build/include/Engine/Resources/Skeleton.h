#pragma once

#include "Math/Vectors.h"
#include "Math/Transform.h"
#include "Skeleton.generated.h"

STRUCT()
struct FBone
{
	GENERATED_BODY()

public:
	PROPERTY(Editable)
	FString name;

	PROPERTY()
	uint32 parent = -1;

	PROPERTY(Editable)
	FVector position;

	PROPERTY(Editable)
	FQuaternion rotation;
};

STRUCT()
struct FSkeleton
{
	GENERATED_BODY()

public:
	// inverse model matrices of all bones
	TArray<FMatrix> invModel; 

	PROPERTY(Editable)
	TArray<FBone> bones;
};

// helper class for transforming skeletons
STRUCT()
struct FSkeletonInstance
{
	GENERATED_BODY()

public:
	PROPERTY(Editable)
	TArray<FTransform> bones;
};
