#pragma once

#include "EngineCore.h"
#include "Vectors.h"
#include "Spline.generated.h"

STRUCT()
struct ENGINE_API FSplinePoint
{
	GENERATED_BODY()

public:
	FVector position;

	FVector controlPointA;
	FVector controlPointB;
};

STRUCT()
struct ENGINE_API FSpline
{
	GENERATED_BODY()

public:
	// returns the index
	uint AddPoint(const FSplinePoint&);

	bool RemovePoint(uint index);

	float ComputeLength(int sampleCount = 16);

	FSplinePoint* GetPoint(uint index);

	FVector SampleAtT(float t);
	FVector SampleAtDistance(float distance);

	FVector TangentAtT(float t);

	float GetTFromDistance(float distance);

public:
	inline SizeType PointsCount() const { return points.Size(); }

	inline float GetLength() const { return totalLength; }
	inline float GetLenghtAtIndex(uint i) const { if (i < lengthLUT.Size()) return lengthLUT[i]; return 0.f; }

private:
	TArray<FSplinePoint> points;
	TArray<float> lengthLUT;

	float totalLength = -1.f;
};
