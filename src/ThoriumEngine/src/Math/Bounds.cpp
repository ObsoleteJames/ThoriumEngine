
#include "Bounds.h"
#include "Math/Transform.h"

FBounds::FBounds(const FVector& pos, const FVector& ext) : position(pos), extents(ext)
{
}

FBounds FBounds::FromMinMax(const FVector& min, const FVector& max)
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

bool FBounds::IsOverlapping(const FBounds& other) const
{
	FVector min = Min();
	FVector max = Max();

	FVector oMin = other.Min();
	FVector oMax = other.Max();

	return	min.x <= oMax.x && 
			max.x >= oMax.x &&
			min.y <= oMax.y &&
			max.y >= oMax.y &&
			min.z <= oMax.z &&
			max.z >= oMax.z;
}

bool FBounds::IsInside(const FBounds& b) const
{
	return b.Min() > Min() && b.Max() < Max();
}

bool FBounds::IsInside(const FVector& point) const
{
	return point > Min() && point < Max();
}

float FBounds::Distance(const FBounds& b) const
{
	return FVector::Distance(Clamp(b.position), b.Clamp(position));
}

float FBounds::Distance(const FVector& point) const
{
	return FVector::Distance(Clamp(point), point);
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

	r = r.Rotate(t.rotation, pivot);

	r.position += t.position;
	return r;
}
