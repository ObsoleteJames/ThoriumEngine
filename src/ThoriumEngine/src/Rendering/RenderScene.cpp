
#include "RenderScene.h"
#include "RenderProxies.h"
#include "Math/Transform.h"
#include "Renderer.h"

CRenderScene::CRenderScene(int fbWidth /*= 1280*/, int fbHeight /*= 720*/)
{
	primaryCamera = nullptr;

	bufferWidth = fbWidth;
	bufferHeight = fbHeight;

	colorBuffer = gRenderer->CreateFrameBuffer(fbWidth, fbHeight, TEXTURE_FORMAT_RGBA16_FLOAT);
	GBufferA = gRenderer->CreateFrameBuffer(fbWidth, fbHeight, TEXTURE_FORMAT_R10G10B10A2_UNORM);
	GBufferB = gRenderer->CreateFrameBuffer(fbWidth, fbHeight, TEXTURE_FORMAT_RGBA8_UNORM);
	GBufferC = gRenderer->CreateFrameBuffer(fbWidth, fbHeight, TEXTURE_FORMAT_RGBA8_UNORM);
	GBufferD = gRenderer->CreateFrameBuffer(fbWidth, fbHeight, TEXTURE_FORMAT_RGBA8_UNORM);

	preTranslucentBuff = gRenderer->CreateFrameBuffer(fbWidth, fbHeight, TEXTURE_FORMAT_RGBA16_FLOAT);

	aoBuffer = gRenderer->CreateFrameBuffer(fbWidth / 2, fbHeight / 2, TEXTURE_FORMAT_R8_UNORM);

	depth = gRenderer->CreateDepthBuffer({ fbWidth, fbHeight, TH_DBF_D24_S8, 1, false });
}

CRenderScene::~CRenderScene()
{
	delete colorBuffer;
	delete GBufferA;
	delete GBufferB;
	delete GBufferC;
	delete GBufferD;
	delete preTranslucentBuff;
	delete aoBuffer;
	delete depth;
}

void CRenderScene::ResizeBuffers(int width, int height)
{
	if (width == bufferWidth && height == bufferHeight)
		return;

	bufferWidth = width;
	bufferHeight = height;

	colorBuffer->Resize(width, height);
	GBufferA->Resize(width, height);
	GBufferB->Resize(width, height);
	GBufferC->Resize(width, height);
	GBufferD->Resize(width, height);

	preTranslucentBuff->Resize(width, height);

	aoBuffer->Resize(width / 2, height / 2);

	depth->Resize(width, height);
}

bool CRenderScene::RayCast(const FVector& raypos, const FVector& dir, FPrimitiveHitInfo* outHit, float maxDistance /*= 0.f*/)
{
	float closesthit = maxDistance > 0.f ? maxDistance : FLT_MAX;
	bool r = false;

	// this is fucking disgusting. kill yourself.
	// TODO: partition the primitives for optimization.
	for (auto* p : primitives)
	{
		FMeshBuilder mb;
		p->GetDynamicMeshes(mb);

		// bounds raycast output
		FVector bPos;
		FVector bNormal;

		if (!FMath::RayAABB(p->Bounds(), FRay(raypos, dir), &bPos, &bNormal))
			continue;

		bool bHadMeshes = false;
		for (auto& m : mb.GetMeshes())
		{
			FMesh& mesh = m.mesh;
			if (!mesh.vertexData || mesh.topologyType != FMesh::TOPOLOGY_TRIANGLES)
				continue;

			bHadMeshes = true;

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

		// if the primitive had no proper mesh data, use the bounds as the raycast target instead
		if (!bHadMeshes)
		{
			float dist = FVector::Distance(bPos, raypos);
			if (dist < closesthit && dist > 0.f)
			{
				r = true;
				closesthit = dist;
				if (outHit)
				{
					outHit->distance = dist;
					outHit->hitProxy = p;
					outHit->normal = bNormal;
					outHit->position = bPos;
					outHit->hitFace = -1;
					outHit->materialIndex = -1;
				}
			}
		}
	}

	return r;
}

bool CRenderScene::RayCastBounds(const FRay& ray, FPrimitiveHitInfo* outHit, float maxDistance /*= 0.f*/)
{
	float closesthit = maxDistance > 0.f ? maxDistance : FLT_MAX;
	bool r = false;

	for (auto* p : primitives)
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

void CRenderScene::GeneratePrimitiveGraph()
{

}
