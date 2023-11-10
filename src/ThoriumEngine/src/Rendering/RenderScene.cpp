
#include "RenderScene.h"
#include "RenderProxies.h"
#include "Math/Transform.h"

bool CRenderScene::RayCast(const FVector& raypos, const FVector& dir, FPrimitiveHitInfo* outHit, float maxDistance /*= 0.f*/)
{
	float closesthit = FLT_MAX;
	bool r = false;

	// this is fucking disgusting. who ever wrote this, kill yourself.
	// TODO: partition the primitives for optimization.
	for (auto* p : primitves)
	{
		FMeshBuilder mb;
		p->GetDynamicMeshes(mb);

		if (!FMath::RayAABB(p->Bounds(), FRay(raypos, dir)))
			continue;

		for (auto& m : mb.GetMeshes())
		{
			FMesh& mesh = m.mesh;
			if (!mesh.vertexData || mesh.topologyType != FMesh::TOPOLOGY_TRIANGLES)
				continue;

			uint faceCount = mesh.numIndexData / 3;
			for (int i = 0; i < faceCount; i++)
			{
				uint i0 = mesh.indexData[(i * 3)];
				uint i1 = mesh.indexData[(i * 3) + 1];
				uint i2 = mesh.indexData[(i * 3) + 2];
				if (i0 > mesh.numVertexData)
					continue;

				FVector v0 = mesh.vertexData[i0].position;
				FVector v1 = mesh.vertexData[i1].position;
				FVector v2 = mesh.vertexData[i2].position;

				// Transform the vertices
				v0 = v0 * m.transform;
				v1 = v1 * m.transform;
				v2 = v2 * m.transform;

				float dist;
				FVector pos;
				FVector normal;

				bool bHit = FMath::RayTriangle(v0, v1, v2, FRay(raypos, dir), dist, &pos, &normal);
				if (bHit && dist < closesthit && dist > 0.f)
				{
					r = true;
					closesthit = dist;
					if (outHit)
					{
						outHit->distance = dist;
						outHit->hitProxy = p;
						outHit->normal = normal;
						outHit->position = pos;
						outHit->hitFace = i * 3;
						outHit->materialIndex = mesh.materialIndex;
					}
				}
			}
		}
	}

	return r;
}

bool CRenderScene::RayCastBounds(const FRay& ray, FPrimitiveHitInfo* outHit, float maxDistance /*= 0.f*/)
{
	float closesthit = FLT_MAX;
	bool r = false;

	for (auto* p : primitves)
	{
		FMeshBuilder mb;
		p->GetDynamicMeshes(mb);

		FBounds b = p->Bounds();
		if (b.Size().Magnitude() == 0.f)
			continue;

		FVector pos;

		bool bHit = FMath::RayAABB(b, ray, &pos);
		float dist = FVector::Distance(pos, ray.origin);
		if (bHit && dist < closesthit && dist > 0.f)
		{
			r = true;
			closesthit = dist;
			if (outHit)
			{
				outHit->distance = dist;
				outHit->hitProxy = p;
				outHit->position = pos;
			}
		}
	}
	return r;
}
