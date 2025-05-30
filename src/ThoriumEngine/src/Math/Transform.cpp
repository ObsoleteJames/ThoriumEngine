
#include "Transform.h"

FTransform::FTransform(const FVector& pos, const FQuaternion& rot, const FVector& s)
	: position(pos), rotation(rot), scale(s)
{
}

FTransform::FTransform(const FQuaternion& rot) : rotation(rot)
{
}

FTransform FTransform::operator*(const FTransform& t) const
{
	FTransform r;
	r.position = (t.rotation.Rotate(position) * t.scale) + t.position;
	r.rotation = t.rotation * rotation;
	r.scale = scale * t.scale;
	return r;
}

FMatrix FTransform::ToMatrix() const
{
	return (FMatrix(1.f).Translate(position) * rotation).Scale(scale);
}

FTransform FTransform::Lerp(const FTransform& a, const FTransform& b, float t)
{
	return FTransform(
		FVector::Lerp(a.position, b.position, t),
		FQuaternion::Slerp(a.rotation, b.rotation, t),
		FVector::Lerp(a.scale, b.scale, t)
	);
}
