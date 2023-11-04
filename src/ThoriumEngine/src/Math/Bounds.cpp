
#include "Bounds.h"

FBounds::FBounds(const FVector& pos, FVector& ext) : position(pos), extents(ext)
{
}

FBounds FBounds::FromMinMax(FVector min, FVector max)
{
	FVector center = (min + max) / 2;
	FVector extent = (max - min) / 2;

	return FBounds(center, extent);
}

FBounds FBounds::Combine(const FBounds& b) const
{
	FVector aMin = Min();
	FVector aMax = Max();

	if (aMin == 0.f && aMin == aMax)
		return b;

	FVector bMin = b.Min();
	FVector bMax = b.Max();

	FVector rMin;
	FVector rMax;

	rMin.x = aMin.x < bMin.x ? aMin.x : bMin.x;
	rMin.y = aMin.y < bMin.y ? aMin.y : bMin.y;
	rMin.z = aMin.z < bMin.z ? aMin.z : bMin.z;

	rMax.x = aMax.x > bMax.x ? aMax.x : bMax.x;
	rMax.y = aMax.y > bMax.y ? aMax.y : bMax.y;
	rMax.z = aMax.z > bMax.z ? aMax.z : bMax.z;
	return FromMinMax(rMin, rMax);
}

FBounds FBounds::Rotate(const FQuaternion& r, const FVector& pivot) const
{
	FVector maxX = FVector(extents.x * 2, extents.y, 0) + pivot;
	FVector maxY = FVector(0, extents.y * 2, 0) + pivot;
	FVector maxZ = FVector(0, extents.y, extents.z * 2) + pivot;

	maxX = r.Rotate(maxX) - pivot;
	maxY = r.Rotate(maxY) - pivot;
	maxZ = r.Rotate(maxZ) - pivot;

	FBounds b;
	b.extents = { maxX.x / 2, maxY.y / 2, maxZ.z / 2 };
	b.position = position;

	return b;
}
