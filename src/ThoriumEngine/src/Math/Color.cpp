
#include "Color.h"
#include "Math/Math.h"

FColor FColor::white = FColor(1.f, 1.f, 1.f);
FColor FColor::black = FColor();
FColor FColor::grey = FColor(0.5f, 0.5f, 0.5f);
FColor FColor::red = FColor(1.f, 0.f, 0.f);
FColor FColor::orange = UColor(255, 165, 0);
FColor FColor::yellow = FColor(1.f, 1.f, 0.f);
FColor FColor::green = FColor(0.f, 1.f, 0.f);
FColor FColor::cyan = FColor(0.f, 1.f, 1.f);
FColor FColor::blue = FColor(0.f, 0.f, 1.f);
FColor FColor::magenta = FColor(1.f, 0.f, 1.f);
FColor FColor::purple = FColor(0.5f, 0.f, 0.5f);
FColor FColor::pink = UColor(255, 192, 203);
FColor FColor::maroon = FColor(0.5f, 0.f, 0.f);
FColor FColor::brown = UColor(139, 69, 19);
FColor FColor::beige = FColor();
FColor FColor::tan = UColor(210, 180, 140);
FColor FColor::peach = UColor(255, 218, 185);
FColor FColor::lime = UColor(50, 205, 50);
FColor FColor::olive = UColor(109, 113, 46);
FColor FColor::turquoise = UColor(64, 224, 208);
FColor FColor::teal = UColor(0, 128, 128);
FColor FColor::navy_blue = UColor(11, 11, 255);
FColor FColor::indigo = UColor(75, 0, 130);
FColor FColor::violet = UColor(238, 130, 238);

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
