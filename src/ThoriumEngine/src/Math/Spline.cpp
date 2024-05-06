
#include "Spline.h"

uint FSpline::AddPoint(const FSplinePoint& p)
{
	uint index = points.Size();
	points.Add(p);
	return index;
}

bool FSpline::RemovePoint(uint index)
{
	if (index < points.Size())
	{
		points.Erase(points.At(index));
		return true;
	}
	return false;
}

float FSpline::ComputeLength(int sampleCount)
{
	totalLength = 0.f;
	lengthLUT.Clear();
	lengthLUT.Resize(points.Size() - 1);

	for (int i = 0; i < points.Size() - 1; i++)
	{
		for (int ii = 0; ii < sampleCount; ii++)
		{
			float t = (float)ii / sampleCount;
			t += float(i);

			FVector a = SampleAtT(t);
			FVector b = SampleAtT(t + (1.f / sampleCount));

			float distance = FVector::Distance(a, b);
			totalLength += distance;
			lengthLUT[i] += distance;
		}
	}
	return totalLength;
}

FSplinePoint* FSpline::GetPoint(uint index)
{
	if (index < points.Size())
		return &points[index];
	return nullptr;
}

FVector FSpline::SampleAtT(float t)
{
	int index = (int)FMath::Floor(t);

	t = FMath::Mod(t, 1.f);

	const FSplinePoint& pointA = points[index];
	const FSplinePoint& pointB = points[index + 1];

	FVector a = FVector::Lerp(pointA.position, pointA.controlPointB, t);
	FVector b = FVector::Lerp(pointA.controlPointB, pointB.controlPointA, t);
	FVector c = FVector::Lerp(pointB.controlPointA, pointB.position, t);

	FVector d = FVector::Lerp(a, b, t);
	FVector e = FVector::Lerp(b, c, t);

	return FVector::Lerp(d, e, t);
}

FVector FSpline::SampleAtDistance(float distance)
{
	if (float t = GetTFromDistance(distance); t > 0)
		return SampleAtT(t);

	return FVector::zero;
}

FVector FSpline::TangentAtT(float t)
{
	return FVector::zero;
}

float FSpline::GetTFromDistance(float distance)
{
	float distanceCovered = 0.f;
	for (int i = 0; i < lengthLUT.Size(); i++)
	{
		if (distance > distanceCovered && distance < distanceCovered + lengthLUT[i])
		{
			float t = (distance - distanceCovered) / lengthLUT[i];
			return t + float(i);
		}

		distanceCovered += lengthLUT[i];
	}
	return -1.f;
}
