#pragma once

#include "EngineCore.h"
#include "Object/Object.h"

#include "Color.generated.h"

STRUCT()
struct ENGINE_API FColor
{
	GENERATED_BODY()

public:
	static FColor white;
	static FColor black;
	static FColor grey;

	static FColor red;
	static FColor orange;
	static FColor yellow;
	static FColor green;
	static FColor cyan;
	static FColor blue;
	static FColor magenta;
	static FColor purple;
	static FColor pink;
	static FColor maroon;
	static FColor brown;
	static FColor beige;
	static FColor tan;
	static FColor peach;
	static FColor lime;
	static FColor olive;
	static FColor turquoise;
	static FColor teal;
	static FColor navy_blue;
	static FColor indigo;
	static FColor violet;

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

// create an FColor from values ranged 0 - 255
inline FColor UColor(uint8 r, uint8 g, uint8 b, uint8 a = 255) { return FColor(float(r) / 255.f, float(g) / 255.f, float(b) / 255.f, float(a) / 255.f); }
