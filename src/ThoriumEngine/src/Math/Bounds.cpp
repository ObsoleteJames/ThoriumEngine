
#include "Bounds.h"
#include "Math/Transform.h"

FBounds::FBounds(const FVector& pos, FVector& ext) : position(pos), extents(ext)
{
}

FBounds FBounds::FromMinMax(FVector min, FVector max)
{
	FVector center = (min + max) / 2;
	FVector extent = (max - min) / 2;

	return FBounds(center, extent);
}

FVector FBounds::Clamp(const FVector& point) const
{
	FVector min = Min();
	FVector max = Max();
	return FVector(FMath::Clamp(point.x, min.x, max.x), FMath::Clamp(point.y, min.y, max.y), FMath::Clamp(point.z, min.z, max.z));
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
	FVector nMin;
	FVector nMax;

	FVector min = Min();
	FVector max = Max();

	FVector verts[8] = {
		min,
		FVector(min.x, min.y, max.z),
		FVector(max.x, min.y, max.z),
		FVector(max.x, min.y, min.z),
		max,
		FVector(max.x, max.y, min.z),
		FVector(min.x, max.y, min.z),
		FVector(min.x, max.y, max.z)
	};

	for (int i = 0; i < 8; i++)
		verts[i] = r.Rotate(verts[i] - pivot) + pivot;

	for (int i = 0; i < 8; i++)
	{
		FVector& v = verts[i];

		if (v.x < nMin.x)
			nMin.x = v.x;
		if (v.y < nMin.y)
			nMin.y = v.y;
		if (v.z < nMin.z)
			nMin.z = v.z;

		if (v.x > nMax.x)
			nMax.x = v.x;
		if (v.y > nMax.y)
			nMax.y = v.y;
		if (v.z > nMax.z)
			nMax.z = v.z;
	}

	return FromMinMax(nMin, nMax);
}

FBounds FBounds::operator*(const FTransform& t)
{
	FBounds r = *this;

	r.extents *= t.scale;

	FVector pivot = r.position - t.position;

	FVector maxX = FVector(r.extents.x * 2, r.extents.y, 0) - pivot;
	FVector maxY = FVector(0, r.extents.y * 2, 0) - pivot;
	FVector maxZ = FVector(0, r.extents.y, r.extents.z * 2) - pivot;

	maxX = t.rotation.Rotate(maxX) + pivot;
	maxY = t.rotation.Rotate(maxY) + pivot;
	maxZ = t.rotation.Rotate(maxZ) + pivot;

	r.extents.x = maxX.x > maxY.x ? (maxX.x > maxZ.x ? maxX.x : maxZ.x) : maxY.x;
	r.extents.y = maxX.y > maxY.y ? (maxX.y > maxZ.y ? maxX.y : maxZ.y) : maxY.y;
	r.extents.z = maxX.z > maxY.z ? (maxX.z > maxZ.z ? maxX.z : maxZ.z) : maxY.z;

	r.position += t.position;
	return r;
}
