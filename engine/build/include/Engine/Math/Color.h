#pragma once

#include "EngineCore.h"
#include "Object/Object.h"

#include "Color.generated.h"

STRUCT()
struct ENGINE_API FColor
{
	GENERATED_BODY()

public:
	FColor();
	FColor(float red, float green, float blue, float alpha = 1.f);
	
	static FColor Hue(float h);
	static FColor HSV(float h, float s, float v);

	inline FColor WithR(float v) const { return FColor(v, g, b, a); }
	inline FColor WithG(float v) const { return FColor(r, v, b, a); }
	inline FColor WithB(float v) const { return FColor(r, g, v, a); }
	inline FColor WithAlpha(float v) const { return FColor(r, g, b, v); }

	inline FColor Inverted() const { return FColor(1.f - r, 1.f - g, 1.f - b, a); }

	inline FColor operator*(const FColor& v) const { return FColor(r * v.r, g * v.g, b * v.b, a * v.a); }
	inline FColor operator/(const FColor& v) const { return FColor(r / v.r, g / v.g, b / v.b, a / v.a); }
	inline FColor operator+(const FColor& v) const { return FColor(r + v.r, g + v.g, b + v.b, a); }
	inline FColor operator-(const FColor& v) const { return FColor(r - v.r, g - v.g, b - v.b, a); }

	inline FColor operator*(float v) const { return FColor(r * v, g * v, b * v, a); }
	inline FColor operator/(float v) const { return FColor(r / v, g / v, b / v, a); }
	inline FColor operator+(float v) const { return FColor(r + v, g + v, b + v, a); }
	inline FColor operator-(float v) const { return FColor(r - v, g - v, b - v, a); }

public:
	PROPERTY(Editable)
	float r;

	PROPERTY(Editable)
	float g;

	PROPERTY(Editable)
	float b;

	PROPERTY(Editable)
	float a;
};
