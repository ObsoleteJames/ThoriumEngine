#pragma once

#include "Math/Vectors.h"
#include "Math/Transform.h"
#include "Bounds.generated.h"

STRUCT()
struct ENGINE_API FBounds
{
	GENERATED_BODY()

public:
	FBounds() = default;
	FBounds(const FBounds&) = default;
	FBounds(const FVector& pos, const FVector& ext);

	static FBounds FromMinMax(const FVector& min, const FVector& max);

	inline FVector Min() const { return position - extents; }
	inline FVector Max() const { return position + extents; }
	inline FVector Size() const { return extents * 2; }

	FVector Clamp(const FVector& point) const;

	// wether this bounding box is overlapping another.
	bool IsOverlapping(const FBounds&) const;

	// wether the given bounding box is inside this.
	bool IsInside(const FBounds&) const;
	bool IsInside(const FVector& point) const;

	float Distance(const FBounds&) const;
	float Distance(const FVector& point) const;

	FBounds Combine(const FBounds& b) const;

	FBounds Rotate(const FQuaternion& r, const FVector& pivot = FVector()) const;

	FBounds operator*(const FTransform& t);

public:
	FVector position;
	FVector extents;
};
