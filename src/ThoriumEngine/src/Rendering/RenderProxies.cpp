
#include "RenderProxies.h"
#include "GraphicsInterface.h"
#include "RenderScene.h"
#include "Renderer.h"
#include "Assets/Material.h"
#include "PostProcessing.h"

//CCameraProxy::~CCameraProxy()
//{
//	scene->UnregisterCamera(this);
//}

void CCameraProxy::CalculateMatrix(float aspectRatio)
{
	view = FMatrix(1.f).Translate(position) * rotation;
	view = view.Inverse();
	if (bOrthographic)
	{
		float halfHeight = FMath::Radians(fov);
		float halfWidth = halfHeight * aspectRatio;
		projection = FMatrix::Orthographic(-halfWidth, halfWidth, -halfHeight, halfHeight, nearPlane, farPlane);
	}
	else
		projection = FMatrix::Perspective(FMath::Radians(fov), aspectRatio, nearPlane, farPlane);
}

FVector2 CCameraProxy::WorldSpaceToScreenPos(const FVector& position, const FVector2& screenSize)
{
	FMatrix pv = projection * view;
	glm::vec4 clipPos = (glm::mat4)pv * glm::vec4((glm::vec3)position, 1.0f);

	glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;

	FVector2 r {
		(ndc.x + 1.0f) * 0.5f * screenSize.x,
		(1.0f - ndc.y) * 0.5f * screenSize.y,
	};
	return r;
}

FMeshBuilder::FMeshBuilder(TArray<FRenderMesh>* output) : meshes(output)
{
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

	FBufferDescriptor desc{};
	desc.bufferSize = mesh.numVertices * sizeof(FVertex);
	desc.data = verts.Data();
	desc.dataStride = sizeof(FVertex);
	desc.flags = 0;
	desc.type = TH_BUFFER_TYPE_VERTEX_BUFFER;

	mesh.vertexBuffer = gGHI->CreateBuffer(desc);
	mesh.topologyType = FMesh::TOPOLOGY_LINES;
	
	FRenderMesh rm;
	rm.mesh = mesh;
	rm.mat = CreateObject<CMaterial>();
	rm.mat->SetShader("Tools");
	rm.transform = FMatrix(1.f);
	rm.rp = rm.mat->GetRenderPass();

	meshes->Add(rm);
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

	FBufferDescriptor desc{};
	desc.bufferSize = mesh.numVertices * sizeof(FVertex);
	desc.data = verts.Data();
	desc.dataStride = sizeof(FVertex);
	desc.flags = 0;
	desc.type = TH_BUFFER_TYPE_VERTEX_BUFFER;

	mesh.vertexBuffer = gGHI->CreateBuffer(desc);
	mesh.topologyType = FMesh::TOPOLOGY_LINES;

	FRenderMesh rm;
	rm.mesh = mesh;
	rm.mat = CreateObject<CMaterial>();
	rm.mat->SetShader("Tools");
	rm.transform = FMatrix(1.f);
	rm.rp = rm.mat->GetRenderPass();

	meshes->Add(rm);
}

void FMeshBuilder::DrawSkinnedMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform, const TArray<FMatrix>& skeletonMatrix /*= TArray<FMatrix>()*/)
{
	if (!mat)
		return;

	THORIUM_ASSERT(mesh.bSkinnedMesh, "Cannot draw unskinned mesh as skinned mesh!");

	ERenderPass rp = mat->GetRenderPass();
	meshes->Add({ mesh, mat, transform, (FMatrix*)skeletonMatrix.Data(), skeletonMatrix.Size(), rp});
}

void FMeshBuilder::DrawMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform)
{
	if (!mat)
		return;

	ERenderPass rp = mat->GetRenderPass();
	meshes->Add({ mesh, mat, transform, nullptr, 0, rp });
}

void FMeshBuilder::DrawMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform, int lightmapId, const FVector2& lightmapPos, const FVector2& lightmapScale)
{
	if (!mat)
		return;

	ERenderPass rp = mat->GetRenderPass();
	meshes->Add({ mesh, mat, transform, nullptr, 0, rp, lightmapId, lightmapPos, lightmapScale });
}

bool CPrimitiveProxy::DoFrustumCull(const FMatrix& projection)
{
	FVector min = bounds.Min();
	FVector max = bounds.Max();

	glm::mat4& m = *(glm::mat4*)&projection;

	glm::vec4 row0 = glm::vec4(m[0][0], m[1][0], m[2][0], m[3][0]);
	glm::vec4 row1 = glm::vec4(m[0][1], m[1][1], m[2][1], m[3][1]);
	glm::vec4 row2 = glm::vec4(m[0][2], m[1][2], m[2][2], m[3][2]);
	glm::vec4 row3 = glm::vec4(m[0][3], m[1][3], m[2][3], m[3][3]);

	// frustum planes
	glm::vec4 planes[6] = {
		row3 + row0, // left
		row3 - row0, // right
		row3 + row1, // bottom
		row3 - row1, // top
		row3 + row2, // near
		row3 - row2, // far
	};

	for (int i = 0; i < 6; ++i)
	{
		glm::vec3 n = glm::vec3(planes[i]);
		float len = glm::length(n);
		if (len > 0.0f)
			planes[i] /= len;
	}

	// AABB vs frustum-plane test using the "positive vertex" method.
	for (int i = 0; i < 6; ++i)
	{
		glm::vec3 n = glm::vec3(planes[i]);
		float d = planes[i].w;

		// Choose the vertex of the AABB that is most likely to be outside (positive vertex)
		glm::vec3 p;
		p.x = (n.x >= 0.0f) ? max.x : min.x;
		p.y = (n.y >= 0.0f) ? max.y : min.y;
		p.z = (n.z >= 0.0f) ? max.z : min.z;

		// If the positive vertex is outside this plane, the whole AABB is outside.
		if (glm::dot(n, p) + d < 0.0f)
			return false;
	}

	// Not outside any plane -> visible / intersects frustum
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
