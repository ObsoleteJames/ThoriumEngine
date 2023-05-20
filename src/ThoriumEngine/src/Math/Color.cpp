
#include "Color.h"
#include "Math/Math.h"

FColor::FColor() : r(0.f), g(0.f), b(0.f), a(1.f)
{
}

FColor::FColor(float red, float green, float blue, float alpha /*= 1.f*/)
	: r(red), g(green), b(blue), a(alpha)
{
}

FColor FColor::Hue(float h)
{
	FColor r;
	r.r = FMath::Saturate(FMath::Abs(h * 6 - 3) - 1);
	r.g = FMath::Saturate(2 - FMath::Abs(h * 6 - 2));
	r.b = FMath::Saturate(2 - FMath::Abs(h * 6 - 4));
	return r;
}

FColor FColor::HSV(float h, float s, float v)
{
	FColor r = Hue(h);
	float C = (1 - FMath::Abs(2 * v - 1)) * s;
	return r - 0.5f * C + v;
}
