#pragma once

#include "EngineCore.h"
#include <math.h>
#include <cmath>

struct FVector;
struct FRay;
struct FBounds;
struct FQuaternion;

class ENGINE_API FMath
{
public:
	static SizeType Random64();

	static uint Random();
	static uint Random(uint range);
	static uint Random(uint min, uint max);

	template<typename T>
	static inline T Lerp(T a, T b, T t) { return ((T)1.0 - t) * a + t * b; }

	template<typename T>
	static inline T Cos(T f) { return std::cos(f); }
	template<typename T>
	static inline T Sin(T f) { return std::sin(f); }
	template<typename T>
	static inline T Tan(T f) { return std::tan(f); }
	template<typename T>
	static inline T Acos(T f) { return std::acos(f); }
	template<typename T>
	static inline T Abs(T f) { return std::abs(f); }

	template<typename T>
	static inline T Floor(T f) { return std::floor(f); }
	template<typename T>
	static inline T Ceil(T f) { return std::ceil(f); }

	template<typename T>
	static inline T Mod(T f, T t) { return std::fmod(f, t); }

	template<typename T>
	static T Radians(T degrees) { return degrees * T(0.01745329251994329576923690768489); }

	template<typename T>
	static T Degrees(T radians) { return radians * T(57.295779513082320876798154814105); }

	template<typename T>
	static inline T Sqrt(T in) { return std::sqrt(in); }

	template<typename T>
	static inline T Squared(const T& in) { return in * in; }

	template<typename T>
	static FString ToString(T number, uint8_t base = 10);

	// Clamp between 0 and 1
	template<typename T>
	static inline T Saturate(T value) { return Clamp(value, T(0), T(1)); }

	template<typename T>
	static inline T Clamp(T value, T min, T max);
	template<typename T>
	static inline T Min(T a, T b);
	template<typename T>
	static inline T Max(T a, T b);

	static void RayCylinderIntersection(const FVector& cylinderCenter, const FVector& cylinderDir, double cylinderRadius, double cylinderHeight,
		const FVector& rayOrigin, const FVector& rayDir, bool& bIntersects, double& out);
	static bool RayTriangle(const FVector& v0, const FVector& v1, const FVector& v2, const FRay& ray, float& dist, FVector* outPos = nullptr, FVector* outNormal = nullptr);
	static bool RaySphere(const FVector& pos, float radius, const FRay& ray, FVector* outPos = nullptr, FVector* outNormal = nullptr);
	static bool RayAABB(const FBounds& aabb, const FRay& ray, FVector* outPos = nullptr, FVector* outNormal = nullptr);
	static bool RayBox(const FBounds& box, const FQuaternion& rot, const FRay& ray, FVector* outPos = nullptr, FVector* outNormal = nullptr);
};

template<typename T>
FString FMath::ToString(T number, uint8_t base /*= 10*/)
{
	static_assert(std::is_integral<T>::value);
	constexpr char charTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

	FString r;
	base = FMath::Clamp<T>(base, 2, 16);

	while (number != 0)
	{
		r += charTable[number % base];
		number /= base;
	}

	if (base == 16)
		r += "x0";
	else if (base == 2)
		r += "b0";

	r.Reverse();
	return r;
}

template<typename T>
T FMath::Clamp(T value, T min, T max)
{
	return value < min ? min : (value > max ? max : value);
}

template<typename T>
T FMath::Min(T a, T b)
{
	return a > b ? b : a;
}

template<typename T>
T FMath::Max(T a, T b)
{
	return a > b ? a : b;
}
