#pragma once

#include "Math/Vectors.h"
#include "Object/Object.h"

#include "Transform.generated.h"

STRUCT()
class ENGINE_API FTransform
{
	GENERATED_BODY()

public:
	FTransform() = default;
	FTransform(const FVector& pos, const FQuaternion& rot = FQuaternion(), const FVector& s = FVector(1.f));
	FTransform(const FQuaternion& rot);

	// Add position
	inline FTransform operator+(const FVector& pos) const { return FTransform(position + pos, rotation, scale); }

	// Subtract position
	inline FTransform operator-(const FVector& pos) const { return FTransform(position - pos, rotation, scale); }

	// Multiply scale
	inline FTransform operator*(const FVector& s) const { return FTransform(position, rotation, scale * s); }

	// Divide scale
	inline FTransform operator/(const FVector& s) const { return FTransform(position, rotation, scale / s); }

	// Rotate
	inline FTransform operator*(const FQuaternion& rot) const { return FTransform(position, rotation * rot, scale); }

	// Add the two transforms together
	FTransform operator*(const FTransform&) const;

	FMatrix ToMatrix() const;

public:
	PROPERTY()
	FVector position;

	PROPERTY()
	FQuaternion rotation;

	PROPERTY()
	FVector scale = FVector(1.f);
};
