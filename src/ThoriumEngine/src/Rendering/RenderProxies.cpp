
#include "RenderProxies.h"
#include "RenderScene.h"
#include "Renderer.h"
#include "Resources/Material.h"
#include "PostProcessing.h"

//CCameraProxy::~CCameraProxy()
//{
//	scene->UnregisterCamera(this);
//}

void CCameraProxy::CalculateMatrix(float aspectRatio)
{
	view = FMatrix(1.f).Translate(position) * rotation;
	view = view.Inverse();
	if (!bOrthographic)
		projection = FMatrix::Perspective(FMath::Radians(fov), aspectRatio, nearPlane, farPlane);
	else
		projection = FMatrix::Orthographic(-(fov * aspectRatio), fov * aspectRatio, -(fov), fov, nearPlane, farPlane);
}

void FMeshBuilder::DrawLine(const FVector& begin, const FVector& end, const FVector& color /*= { 255, 255, 255 }*/, bool bDepthTest)
{
	TArray<FVertex> verts(2);
	
	verts[0].position = begin;
	verts[1].position = end;

	verts[0].color = color;
	verts[1].color = color;

	FMesh mesh;
	mesh.numVertices = 2;
	mesh.vertexBuffer = gRenderer->CreateVertexBuffer(verts);
	mesh.topologyType = FMesh::TOPOLOGY_LINES;
	
	FRenderMesh rm;
	rm.mesh = mesh;
	rm.mat = CreateObject<CMaterial>();
	rm.mat->SetShader("Tools");
	rm.transform = FMatrix(1.f);
	rm.rp = rm.mat->GetRenderPass();

	meshes.Add(rm);
}

void FMeshBuilder::DrawCircle(const FVector& pos, float radius /*= 1.f*/, const FVector& rot /*= FVector()*/, const FVector& color /*= { 255, 255, 255 }*/, int vertices, bool bDepthTest)
{
	FQuaternion qRot = FQuaternion::EulerAngles(rot);

	TArray<FVertex> verts;

	for (int i = 0; i < vertices; i++)
	{
		for (int y = 0; y < 2; y++)
		{
			float degree = (float)(i + y) / (float)vertices;
			degree *= 360.f;
			float r = FMath::Radians(degree);

			float x = FMath::Sin(r);
			float z = FMath::Cos(r);

			FVertex vert{};
			vert.position = FVector(x, 0.f, z);
			vert.color = color;
			
			vert.position = qRot.Rotate(vert.position);
			vert.position *= radius;
			vert.position += pos;
			
			verts.Add(vert);
		}
	}
	// Make sure it loops.
	verts.Add(verts[0]);

	FMesh mesh;
	mesh.numVertices = verts.Size();
	mesh.vertexBuffer = gRenderer->CreateVertexBuffer(verts);
	mesh.topologyType = FMesh::TOPOLOGY_LINES;

	FRenderMesh rm;
	rm.mesh = mesh;
	rm.mat = CreateObject<CMaterial>();
	rm.mat->SetShader("Tools");
	rm.transform = FMatrix(1.f);
	rm.rp = rm.mat->GetRenderPass();

	meshes.Add(rm);
}

void FMeshBuilder::DrawMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform, const TArray<FMatrix>& skeletonMatrix /*= TArray<FMatrix>()*/)
{
	if (!mat)
		return;

	ERenderPass rp = mat->GetRenderPass();
	meshes.Add({ mesh, mat, transform, skeletonMatrix, rp });
}

bool CPrimitiveProxy::DoFrustumCull(CCameraProxy* cam)
{
	return true;
}

bool CPostProcessVolumeProxy::IsCameraInsideVolume(CCameraProxy* proxy) const
{
	FVector camPos = proxy->position - bounds.position;
	camPos = rotation.Rotate(camPos) + bounds.position;

	FVector min = bounds.Min();
	FVector max = bounds.Max();

	if (camPos.x > min.x && camPos.x < max.x &&
		camPos.y > min.y && camPos.y < max.y &&
		camPos.z > min.z && camPos.z < max.z)
		return true;

	return false;
}

float CPostProcessVolumeProxy::GetInfluence(CCameraProxy* proxy) const
{
	FVector camPos = proxy->position - bounds.position;
	camPos = rotation.Rotate(camPos) + bounds.position;

	FVector min = bounds.Min();
	FVector max = bounds.Max();

	float distanceFromEdge = FLT_MAX;

	float distMinX = FMath::Abs(min.x - camPos.x);
	float distMinY = FMath::Abs(min.y - camPos.y);
	float distMinZ = FMath::Abs(min.z - camPos.z);

	float distMaxX = FMath::Abs(max.x - camPos.x);
	float distMaxY = FMath::Abs(max.y - camPos.y);
	float distMaxZ = FMath::Abs(max.z - camPos.z);

	if (distMinX < distMaxX)
	{
		if (distMinX < distanceFromEdge)
			distanceFromEdge = distMinX;
	}
	else
	{
		if (distMaxX < distanceFromEdge)
			distanceFromEdge = distMaxX;
	}

	if (distMinY < distMaxY)
	{
		if (distMinY < distanceFromEdge)
			distanceFromEdge = distMinY;
	}
	else
	{
		if (distMaxY < distanceFromEdge)
			distanceFromEdge = distMaxY;
	}

	if (distMinZ < distMaxZ)
	{
		if (distMinZ < distanceFromEdge)
			distanceFromEdge = distMinZ;
	}
	else
	{
		if (distMaxZ < distanceFromEdge)
			distanceFromEdge = distMaxZ;
	}

	return FMath::Clamp(distanceFromEdge / fade, 0.f, 1.f);
}
