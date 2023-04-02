
#include "Vectors.h"
#include "Rendering/RenderProxies.h"
#include "Window.h"
#include <DirectXMath.h>

FVector FVector::Orthogonal() const
{
	FVector absVec(FMath::Abs(x), FMath::Abs(y), FMath::Abs(z));
	if ((absVec.x <= absVec.y) && (absVec.x <= absVec.z))
		return FVector(0, z, -y);
	if ((absVec.z <= absVec.x) && (absVec.z <= absVec.y))
		return FVector(y, -x, 0);
	return FVector(-z, 0, x);
}

float FVector::Distance(const FVector& a, const FVector& b)
{
	return FMath::Sqrt(FMath::Squared(b.x - a.x) + FMath::Squared(b.y - a.y) + FMath::Squared(b.z - a.z));
}

float FVector::Dot(const FVector& a, const FVector& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

FVector FVector::Cross(const FVector& a, const FVector& b)
{
	FVector r;
	r.x = a.y * b.z - b.y * a.z;
	r.y = a.z * b.x - b.z * a.x;
	r.z = a.x * b.y - b.x * a.y;
	return r;
}

FVector& FVector::operator-=(const FVector& right)
{
	x -= right.x;
	y -= right.y;
	z -= right.z;
	return *this;
}

FVector& FVector::operator*=(const FVector& right)
{
	x *= right.x;
	y *= right.y;
	z *= right.z;
	return *this;
}

FVector& FVector::operator/=(const FVector& right)
{
	x /= right.x;
	y /= right.y;
	z /= right.z;
	return *this;
}

FVector& FVector::operator+=(const FVector& right)
{
	x += right.x;
	y += right.y;
	z += right.z;
	return *this;
}

FVector& FVector::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

FVector& FVector::operator/=(float f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

FQuaternion FQuaternion::EulerAngles(const FVector& in)
{
	glm::quat qX = glm::angleAxis(in.x, glm::vec3(1, 0, 0));
	glm::quat qY = glm::angleAxis(in.y, glm::vec3(0, 1, 0));
	glm::quat qZ = glm::angleAxis(in.z, glm::vec3(0, 0, 1));
	return (FQuaternion)(qY * qX * qZ);
}

FQuaternion FQuaternion::Normalized() const
{
	FQuaternion r(*this);
	float mag = r.Magnitude();
	r.x /= mag;
	r.y /= mag;
	r.z /= mag;
	r.w /= mag;
	return r;
}

FVector FQuaternion::Rotate(const FVector& r) const
{
	FVector quatVec(x, y, z);
	FVector uv = FVector::Cross(quatVec, r);
	FVector uuv = FVector::Cross(quatVec, uv);

	return r + ((uv * w) + uuv) * 2.f;
}

FQuaternion& FQuaternion::operator+=(const FQuaternion& right)
{
	glm::quat q = (glm::quat)*this += (glm::quat)right;
	x = q.x;
	y = q.y;
	z = q.z;
	w = q.w;
	return *this;
}

FQuaternion& FQuaternion::operator*=(const FQuaternion& b)
{
	//glm::quat q = (glm::quat)*this *= (glm::quat)r;
	//x = q.x;
	//y = q.y;
	//z = q.z;
	//w = q.w;
	//return *this;

	FQuaternion a = *this;
	w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
	z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
	return *this;
}

FMatrix::FMatrix(const glm::mat4& other)
{
	memcpy(this, &other, sizeof(glm::mat4));
}

FMatrix::FMatrix(float f)
{
	v[0] = f;
	v[5] = f;
	v[10] = f;
	v[15] = f;
}

FMatrix FMatrix::Translate(const FVector& in) const
{
	return (FMatrix)(glm::translate((glm::mat4)*this, (glm::vec3)in));
}

FMatrix FMatrix::Scale(const FVector& in) const
{
	return (FMatrix)(glm::scale((glm::mat4)*this, (glm::vec3)in));
}

FMatrix FMatrix::Inverse() const
{
	return (FMatrix)(glm::inverse((glm::mat4)*this));
}

FMatrix FMatrix::Perspective(float fov, float aspectRatio, float nearPlane, float farPlane)
{
	return (FMatrix)glm::perspectiveLH_NO(fov, aspectRatio, nearPlane, farPlane);
}

FMatrix FMatrix::LookAt(const FVector& pos, const FVector& target, const FVector& up)
{
	return (FMatrix)glm::lookAtLH((glm::vec3)pos, (glm::vec3)target, (glm::vec3)up);
}

FMatrix& FMatrix::operator*=(const FMatrix& other)
{
	(*(glm::mat4*)this) *= (glm::mat4)other;
	return *this;
}

FMatrix& FMatrix::operator*=(const FQuaternion& quat)
{
	(*(glm::mat4*)this) *= glm::toMat4((glm::quat)quat);
	return *this;
}

FQuaternion operator*(const FQuaternion& a, const FQuaternion& b)
{
	FQuaternion r;
	r.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
	r.x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
	r.y = a.w * b.y + a.y * b.w + a.z * b.x - a.x * b.z;
	r.z = a.w * b.z + a.z * b.w + a.x * b.y - a.y * b.x;
	return r;
}

FRay FRay::MouseToRay(CCameraProxy* cam, float x, float y, IBaseWindow* window)
{
	int w, h;
	window->GetSize(w, h);

	y = h - y;

	float mouseX = x / ((float)w * 0.5f) - 1.f;
	float mouseY = y / ((float)h * 0.5f) - 1.f;

	FMatrix invVP = (cam->projection * cam->view).Inverse();
	glm::vec4 screenPos = glm::vec4(mouseX, mouseY, 1.f, 1.f);
	glm::vec4 worldPos = (glm::mat4)invVP * screenPos;

	FVector dir = glm::normalize(glm::vec3(worldPos));

	return { cam->position, dir };
}
