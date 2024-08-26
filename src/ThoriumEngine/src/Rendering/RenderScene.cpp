
#include "RenderScene.h"
#include "RenderProxies.h"
#include "Rendering/GraphicsInterface.h"
#include "Math/Transform.h"
#include "Renderer.h"
#include "Rendering/Texture.h"

static const int bloomScaleLUT[] = {
	2, 4, 8, 16, 32, 64
};

CRenderScene::CRenderScene(int fbWidth /*= 1280*/, int fbHeight /*= 720*/) : primaryCamera(nullptr)
{
	bufferWidth = fbWidth;
	bufferHeight = fbHeight;

	float sp = screenPercentage / 100.f;

	int widthB = int((float)fbWidth * sp);
	int heightB = int((float)fbHeight * sp);

	colorBuffer = gGHI->CreateFrameBuffer(widthB, heightB, TEXTURE_FORMAT_RGBA16_FLOAT, cvRenderFBPointFilter.AsBool() ? THTX_FILTER_POINT : THTX_FILTER_LINEAR);
	GBufferA = gGHI->CreateFrameBuffer(widthB, heightB, TEXTURE_FORMAT_R10G10B10A2_UNORM);
	GBufferB = gGHI->CreateFrameBuffer(widthB, heightB, TEXTURE_FORMAT_RGBA8_UNORM);
	GBufferC = gGHI->CreateFrameBuffer(widthB, heightB, TEXTURE_FORMAT_RGBA8_UNORM);
	GBufferD = gGHI->CreateFrameBuffer(widthB, heightB, TEXTURE_FORMAT_RGBA8_UNORM);

	preTranslucentBuff = gGHI->CreateFrameBuffer(widthB, heightB, TEXTURE_FORMAT_RGBA16_FLOAT);

	aoBuffer = gGHI->CreateFrameBuffer(fbWidth / 2, fbHeight / 2, TEXTURE_FORMAT_R8_UNORM);

	for (int i = 0; i < 4; i++)
	{
		bloomBuffersX[i] = gGHI->CreateFrameBuffer(fbWidth / bloomScaleLUT[i], fbHeight / bloomScaleLUT[i], TEXTURE_FORMAT_RGBA16_FLOAT, THTX_FILTER_LINEAR);
		bloomBuffersY[i] = gGHI->CreateFrameBuffer(fbWidth / bloomScaleLUT[i], fbHeight / bloomScaleLUT[i], TEXTURE_FORMAT_RGBA16_FLOAT, THTX_FILTER_LINEAR);
	}

	depth = gGHI->CreateDepthBuffer({ widthB, heightB, TH_DBF_D24_S8, 1, false });

	depthTex = gGHI->CreateTexture2D(nullptr, widthB, heightB, TEXTURE_FORMAT_R24G8, THTX_FILTER_POINT);
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
	
	for (int i = 0; i < 4; i++)
	{
		delete bloomBuffersX[i];
		delete bloomBuffersY[i];
	}

	delete depth;
	delete depthTex;
}

void CRenderScene::ResizeBuffers(int width, int height)
{
	bufferWidth = width;
	bufferHeight = height;

	float sp = screenPercentage / 100.f;

	int widthB = int((float)width * sp);
	int heightB = int((float)height * sp);

	colorBuffer->Resize(widthB, heightB);
	GBufferA->Resize(widthB, heightB);
	GBufferB->Resize(widthB, heightB);
	GBufferC->Resize(widthB, heightB);
	GBufferD->Resize(widthB, heightB);

	preTranslucentBuff->Resize(widthB, heightB);

	aoBuffer->Resize(width / 2, height / 2);

	for (int i = 0; i < 4; i++)
	{
		bloomBuffersX[i]->Resize(width / bloomScaleLUT[i], height / bloomScaleLUT[i]);
		bloomBuffersY[i]->Resize(width / bloomScaleLUT[i], height / bloomScaleLUT[i]);
	}

	depth->Resize(widthB, heightB);
	delete depthTex;
	depthTex = gGHI->CreateTexture2D(nullptr, widthB, heightB, TEXTURE_FORMAT_R24G8, THTX_FILTER_POINT);
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
