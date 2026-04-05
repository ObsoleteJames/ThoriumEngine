
#include "Transform.h"

FTransform::FTransform(const FVector& pos, const FQuaternion& rot, const FVector& s)
	: position(pos), rotation(rot), scale(s)
{
}

FTransform::FTransform(const FQuaternion& rot) : rotation(rot)
{
}

FTransform::FTransform(const FMatrix& matrix)
{
	matrix.Decompose(position, scale, rotation);
}

FTransform FTransform::operator*(const FTransform& t) const
{
	FTransform r;
	r.position = (t.rotation.Rotate(position) * t.scale) + t.position;
	r.rotation = (t.rotation * rotation).Normalized();
	r.scale = scale * t.scale;
	return r;
}

FTransform FTransform::operator+(const FTransform& t) const
{
	FTransform r;
	r.position = position + t.position;
	r.rotation = (t.rotation * rotation).Normalized();
	r.scale = scale * t.scale;
	return r;
}

FTransform FTransform::Inverse() const
{
	FTransform r;
	r.rotation = rotation.Invert();
	r.scale = FVector(1.f) / scale;
	r.position = -position;
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
