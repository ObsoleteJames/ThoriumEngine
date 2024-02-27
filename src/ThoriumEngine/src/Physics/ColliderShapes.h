#pragma once

#include "Math/Vectors.h"
#include "Math/Transform.h"
#include "ColliderShapes.generated.h"

enum EShapeType
{
	SHAPE_INVALID,
	SHAPE_BOX,
	SHAPE_SPHERE,
	SHAPE_PLANE,
	SHAPE_CAPSULE,
	SHAPE_MESH,
	SHAPE_CONVEX_MESH,
	SHAPE_END
};

STRUCT()
struct ENGINE_API FShapeBox
{
	GENERATED_BODY()

public:
	static inline EShapeType Type() { return SHAPE_BOX; }

public:
	PROPERTY(EditAnywhere)
	FVector center;

	PROPERTY(EditAnywhere)
	FVector size;
};

STRUCT()
struct ENGINE_API FShapeSphere
{
	GENERATED_BODY()

public:
	static inline EShapeType Type() { return SHAPE_SPHERE; }

public:
	PROPERTY(EditAnywhere)
	FVector center;

	PROPERTY(EditAnywhere)
	float radius;
};

STRUCT()
struct ENGINE_API FShapePlane
{
	GENERATED_BODY()

public:
	static inline EShapeType Type() { return SHAPE_PLANE; }

public:
	PROPERTY(EditAnywhere)
	FVector center;

	PROPERTY(EditAnywhere)
	FVector2 size;
};

STRUCT()
struct ENGINE_API FShapeCapsule
{
	GENERATED_BODY()

public:
	static inline EShapeType Type() { return SHAPE_CAPSULE; }

public:
	PROPERTY(EditAnywhere)
	FVector center;

	PROPERTY(EditAnywhere)
	float radius;

	PROPERTY(EditAnywhere)
	float height;
};

STRUCT()
struct ENGINE_API FShapeMesh
{
	GENERATED_BODY()

public:
	PROPERTY()
	TArray<FVector> vertices;

	PROPERTY()
	TArray<uint32> indices;
};
