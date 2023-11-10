
#include "Math.h"
#include "Vectors.h"
#include "Resources/Mesh.h"
#include <random>

static std::random_device _randomDev;
static std::mt19937_64 _engine64(_randomDev());
static std::uniform_int_distribution<SizeType> UniformDist;

SizeType FMath::Random64()
{
	return UniformDist(_engine64);
}

uint FMath::Random(uint range)
{
	return Random() % range;
}

uint FMath::Random(uint min, uint max)
{
	int r = Random();
	r %= max - min;
	r += min;
	return r;
}

uint FMath::Random()
{
	std::uniform_int_distribution<uint> dist;
	return dist(_randomDev);
}

void FMath::RayCylinderIntersection(const FVector& cylinderCenter, const FVector& cylinderDir, double cylinderRadius, double cylinderHeight, const FVector& rayOrigin, const FVector& rayDir, bool& bIntersects, double& out)
{
	bIntersects = false;
	int numIntersections = 0;
	double rayParam[2];

	FVector basis[3];
	basis[0] = cylinderDir.Normalize();
	basis[1] = basis[0].Orthogonal();
	basis[2] = FVector::Cross(basis[0], basis[1]).Normalize();

	double halfHeight = 0.5 * cylinderHeight;
	double radiusSqrd = cylinderRadius * cylinderRadius;

	FVector diff = rayOrigin - cylinderCenter;
	FVector p(FVector::Dot(basis[1], diff), FVector::Dot(basis[2], diff), FVector::Dot(basis[0], diff));

	double Dz = FVector::Dot(basis[0], rayDir);
	if (FMath::Abs(Dz) == 1.0)
	{
		double radialSqrdDist = radiusSqrd - p.x * p.x - p.y * p.y;
		if (radialSqrdDist >= 0.0)
		{
			numIntersections = 2;
			if (Dz > 0.0)
			{
				rayParam[0] = -p.z - halfHeight;
				rayParam[1] = -p.z + halfHeight;
			}
			else
			{
				rayParam[0] = p.z - halfHeight;
				rayParam[1] = p.z + halfHeight;
			}
		}
	}
	else
	{
		FVector d(FVector::Dot(basis[1], rayDir), FVector::Dot(basis[2], rayDir), Dz);

		double A0, A1, A2, discr, root, inv, tValue;

		if (d.z == 0.0)
		{
			if (FMath::Abs(p.z) <= halfHeight)
			{
				A0 = p.x * p.x + p.y * p.y - radiusSqrd;
				A1 = p.x * d.x + p.y * d.y;
				A2 = d.x * d.x + d.y * d.y;
				discr = A1 * A1 - A0 * A2;
				if (discr > 0.0)
				{
					numIntersections = 2;
					root = FMath::Sqrt(discr);
					inv = 1.0 / A2;
					rayParam[0] = (-A1 - root) * inv;
					rayParam[1] = (-A1 + root) * inv;
				}
				else if (discr == 0.0)
				{
					numIntersections = 1;
					rayParam[0] = -A1 / A2;
					rayParam[1] = rayParam[0];
				}
			}
		}
		else
		{
			inv = 1.0 / d.z;

			double T0 = (-halfHeight - p.z) * inv;
			double TmpX = p.x + T0 * d.z;
			double TmpY = p.y + T0 * d.y;
			if (TmpX * TmpX + TmpY * TmpY <= radiusSqrd)
				rayParam[numIntersections++] = T0;

			double T1 = (+halfHeight - p.z) * inv;
			TmpX = p.x + T1 * d.x;
			TmpY = p.y + T1 * d.y;
			if (TmpX * TmpX + TmpY * TmpY <= radiusSqrd)
				rayParam[numIntersections++] = T1;

			if (numIntersections < 2)
			{
				A0 = p.x * p.x + p.y * p.y - radiusSqrd;
				A1 = p.x * d.x + p.y * d.y;
				A2 = d.x * d.x + d.y * d.y;
				discr = A1 * A1 - A0 * A2;
				if (discr > 0.0)
				{
					root = FMath::Sqrt(discr);
					inv = 1.0 / A2;
					tValue = (-A1 - root) * inv;
					if (T0 <= T1)
					{
						if (T0 <= tValue && tValue <= T1)
							rayParam[numIntersections++] = tValue;
					}
					else
					{
						if (T1 <= tValue && tValue <= T0)
							rayParam[numIntersections++] = tValue;
					}

					if (numIntersections < 2)
					{
						tValue = (-A1 + root) * inv;
						if (T0 <= T1)
						{
							if (T0 <= tValue && tValue <= T1)
								rayParam[numIntersections++] = tValue;
						}
						else
						{
							if (T1 <= tValue && tValue <= T0)
								rayParam[numIntersections++] = tValue;
						}
					}
				}
				else if (discr == 0.0)
				{
					tValue = -A1 / A2;
					if (T0 <= T1)
					{
						if (T0 <= tValue && tValue <= T1)
							rayParam[numIntersections++] = tValue;
					}
					else
					{
						if (T1 <= tValue && tValue <= T0)
							rayParam[numIntersections++] = tValue;
					}
				}
			}

			if (numIntersections == 2)
			{
				if (rayParam[0] > rayParam[1])
				{
					double TmpT = rayParam[0];
					rayParam[0] = rayParam[1];
					rayParam[1] = TmpT;
				}
			}
			else if (numIntersections == 1)
			{
				rayParam[1] = rayParam[0];
			}
		}
	}

	if (numIntersections > 0 && rayParam[0] >= 0.0)
	{
		bIntersects = true;
		out = rayParam[0];
	}
	else if (numIntersections == 2 && rayParam[1] >= 0.0)
	{
		bIntersects = true;
		out = rayParam[1];
	}
}

bool FMath::RayTriangle(const FVector& v0, const FVector& v1, const FVector& v2, const FRay& ray, float& t, FVector* outPos /*= nullptr*/, FVector* outNormal /*= nullptr*/)
{
	//---------- METHOD 1 ----------
	//FVector v0v1 = v1 - v0;
	//FVector v0v2 = v2 - v0;
	//FVector N = FVector::Cross(v0v1, v0v2);
	//float area2 = N.Magnitude();

	//float NdotDir = FVector::Dot(N, ray.direction);
	//if (Abs(NdotDir) < 0.0001f)
	//	return false;

	//float d = -FVector::Dot(N, v0);
	//t = -(FVector::Dot(N, ray.origin) + d) / NdotDir;

	//if (t < 0)
	//	return false;

	//FVector P = ray.origin + t * ray.direction;
	//FVector C;

	//FVector edge0 = v1 - v0;
	//FVector vp0 = P - v0;
	//C = FVector::Cross(edge0, vp0);
	//if (FVector::Dot(N, C) < 0)
	//	return false;

	//FVector edge1 = v2 - v1;
	//FVector vp1 = P - v1;
	//C = FVector::Cross(edge1, vp1);
	//if (FVector::Dot(N, C) < 0)
	//	return false;

	//FVector edge2 = v0 - v2;
	//FVector vp2 = P - v2;
	//C = FVector::Cross(edge2, vp2);
	//if (FVector::Dot(N, C) < 0)
	//	return false;

	//if (outPos)
	//	*outPos = ray.origin + ray.direction * t;
	//if (outNormal)
	//	*outNormal = N;

	//return true;

	//---------- METHOD 2 ----------
	FVector v0v1 = v1 - v0;
	FVector v0v2 = v2 - v0;
	FVector pvec = FVector::Cross(ray.direction, v0v2);
	float det = FVector::Dot(v0v1, pvec);

	if (det < 0.000001 && det > -0.0000001)
		return false;

	float invDet = 1.f / det;
	FVector tvec = ray.origin - v0;

	float u = FVector::Dot(tvec, pvec) * invDet;

	if (u < 0.f || u > 1.f)
		return false;

	FVector qvec = FVector::Cross(tvec, v0v1);

	float v = FVector::Dot(ray.direction, qvec) * invDet;

	if (v < 0 || u + v > 1)
		return false;

	t = FVector::Dot(v0v2, qvec) * invDet;
	FVector N = FVector::Cross(v0v1, v0v2);

	if (outPos)
		*outPos = ray.origin + (ray.direction * t);
	if (outNormal)
		*outNormal = N.Normalize();

	return true;
}

bool FMath::RaySphere(const FVector& pos, float radius, const FRay& ray, FVector* outPos /*= nullptr*/, FVector* outNormal /*= nullptr*/)
{
	return false;
}

bool FMath::RayAABB(const FBounds& aabb, const FRay& ray, FVector* outPos /*= nullptr*/, FVector* outNormal /*= nullptr*/)
{
	//FVector p = aabb.Clamp(ray.origin);
	//FVector pd = (p - ray.origin).Normalize();
	//float t = FVector::Dot(pd, ray.direction);

	//if (t > 0.0001 || t < -0.0001)
	//	return false;

	FVector tMin = (aabb.Min() - ray.origin) / ray.direction;
	FVector tMax = (aabb.Max() - ray.origin) / ray.direction;

	FVector t1 = FVector(Min(tMin.x, tMax.x), Min(tMin.y, tMax.y), Min(tMin.z, tMax.z));
	FVector t2 = FVector(Max(tMin.x, tMax.x), Max(tMin.y, tMax.y), Max(tMin.z, tMax.z));

	float tNear = Max(Max(t1.x, t1.y), t1.z);
	float tFar = Min(Min(t2.x, t2.y), t2.z);

	if (tNear > tFar)
		return false;

	if (outPos)
		*outPos = ray.origin + ray.direction * tNear;

	return true;
}

bool FMath::RayBox(const FBounds& box, const FQuaternion& rot, const FRay& ray, FVector* outPos /*= nullptr*/, FVector* outNormal /*= nullptr*/)
{
	FRay r;
	r.origin = rot.Conjugate().Rotate(ray.origin - box.position) + box.position;
	r.direction = rot.Conjugate().Rotate(ray.direction);

	FVector p;
	FVector n;

	bool result = RayAABB(box, r, &p, &n);
	if (result && outPos)
		*outPos = rot.Rotate(p - box.position) + box.position;
	if (result && outNormal)
		*outNormal = rot.Rotate(n);

	return result;
}
