#pragma once

#include "EngineCore.h"
#include "Math.h"
#include "Object/Object.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Vectors.Generated.h"

STRUCT()
struct ENGINE_API FVector
{
	GENERATED_BODY()

public:
	FVector() = default;
	FVector(float scalar) : x(scalar), y(scalar), z(scalar) {}
	FVector(const FVector& other) : x(other.x), y(other.y), z(other.z) {}
	FVector(const glm::vec3& other) : x(other.x), y(other.y), z(other.z) {}
	FVector(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	inline operator glm::vec3() const { return glm::vec3(x, y, z); }

	inline float Magnitude() const { return FMath::Sqrt(x * x + y * y + z * z); }
	inline FVector Normalize() const { return FVector(*this) /= Magnitude(); }
	inline FVector Orthogonal() const;

	static float Distance(const FVector& a, const FVector& b);
	static float Dot(const FVector& a, const FVector& b);
	static FVector Cross(const FVector& a, const FVector& b);

	FVector Radians() const { return FVector(FMath::Radians(x), FMath::Radians(y), FMath::Radians(z)); }
	FVector Degrees() const { return FVector(FMath::Degrees(x), FMath::Degrees(y), FMath::Degrees(z)); }

	FVector Cos() const { return FVector(FMath::Cos(x), FMath::Cos(y), FMath::Cos(z)); }
	FVector Sin() const { return FVector(FMath::Sin(x), FMath::Sin(y), FMath::Sin(z)); }
	FVector Tan() const { return FVector(); }

	inline FVector operator-() { return FVector(-x, -y, -z); }

	FVector& operator+=(const FVector& right);
	FVector& operator-=(const FVector& right);
	FVector& operator*=(const FVector& right);
	FVector& operator/=(const FVector& right);

	FVector& operator*=(float f);
	FVector& operator/=(float f);

	bool operator==(const FVector& right) { return x == right.x && y == right.y && z == right.z; }
	bool operator!=(const FVector& right) { return x != right.x && y != right.y && z != right.z; }

public:
	PROPERTY(Editable)
	float x = 0.0f;
	PROPERTY(Editable)
	float y = 0.0f;
	PROPERTY(Editable)
	float z = 0.0f;
};

inline FVector operator+(const FVector& a, const FVector& b) { return FVector(a) += b; }
inline FVector operator-(const FVector& a, const FVector& b) { return FVector(a) -= b; }
inline FVector operator*(const FVector& a, const FVector& b) { return FVector(a) *= b; }
inline FVector operator/(const FVector& a, const FVector& b) { return FVector(a) /= b; }

inline FVector operator*(const FVector& a, float f) { return FVector(a) *= f; }
inline FVector operator/(const FVector& a, float f) { return FVector(a) /= f; }

STRUCT()
struct ENGINE_API FQuaternion
{
	GENERATED_BODY()

public:
	FQuaternion() = default;
	FQuaternion(const FQuaternion& other) = default;
	FQuaternion(const glm::quat& other) : x(other.x), y(other.y), z(other.z), w(other.w) {}
	FQuaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

	inline operator glm::quat() const { return glm::quat(w, x, y, z); }

	static FQuaternion EulerAngles(const FVector& angle);
	inline FVector ToEuler() const { return (FVector)glm::eulerAngles((glm::quat)*this); }

	inline FQuaternion Conjugate() const { return glm::conjugate((glm::quat)*this); }
	inline FQuaternion Invert() const { return glm::inverse((glm::quat)*this); }

	FVector Rotate(const FVector& r) const;
	inline FVector Forward() const { return Rotate({ 0.f, -1.f, 0.f }); }
	inline FVector Right() const { return Rotate({ 1.f, 0.f, 0.f }); }
	inline FVector Up() const { return Rotate({ 0.f, 0.f, 1.f }); }

	FQuaternion& operator*=(const FQuaternion& right);
	FQuaternion& operator+=(const FQuaternion& right);

	bool operator==(const FQuaternion& right) { return x == right.x && y == right.y && z == right.z && w == right.w; }
	bool operator!=(const FQuaternion& right) { return x != right.x && y != right.y && z != right.z && w != right.w; }

public:
	PROPERTY()
	float x = 0.f;
	PROPERTY()
	float y = 0.f;
	PROPERTY()
	float z = 0.f;
	PROPERTY()
	float w = 1.f;
};

inline FQuaternion operator*(const FQuaternion& a, const FQuaternion& b) { return FQuaternion(a) *= b; }
inline FQuaternion operator+(const FQuaternion& a, const FQuaternion& b) { return FQuaternion(a) += b; }

STRUCT()
struct ENGINE_API FRay
{
	GENERATED_BODY()

public:
	PROPERTY(Editable)
	FVector origin;
	
	PROPERTY(Editable)
	FVector direction;
};

STRUCT()
struct ENGINE_API FMatrix
{
	GENERATED_BODY()

public:
	FMatrix() = default;
	FMatrix(float f);
	FMatrix(const FMatrix&) = default;
	FMatrix(const glm::mat4& other);

	inline operator glm::mat4() const { return glm::mat4(*(glm::mat4*)this); }

	FMatrix Translate(const FVector& in) const;
	FMatrix Scale(const FVector& in) const;
	FMatrix Inverse() const;

	static FMatrix Perspective(float fov, float aspectRatio, float nearPlane, float farPlane);
	static FMatrix LookAt(const FVector& pos, const FVector& target, const FVector& up);

	FMatrix& operator *=(const FMatrix& other);
	FMatrix& operator *=(const FQuaternion& quat);

public:
	float v[16] = { 0.f };
};

inline FMatrix operator*(const FMatrix& a, const FMatrix& b) { return FMatrix(a) *= b; }
inline FMatrix operator*(const FMatrix& a, const FQuaternion& b) { return FMatrix(a) *= b; }
