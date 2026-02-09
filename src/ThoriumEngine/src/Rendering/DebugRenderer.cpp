
#include "DebugRenderer.h"
#include "Assets/Material.h"
#include "Assets/ModelAsset.h"
#include "Renderer.h"
#include "GraphicsInterface.h"
#include "RenderScene.h"
#include "Game/World.h"

//CDebugRenderer* gDebugRenderer;

#define DR_VERTEX_LINEBUFFER_SIZE sizeof(FVertex) * 512
#define DR_VERTEX_SOLIDBUFFER_SIZE sizeof(FVertex) * 4096

CDebugRenderer::CDebugRenderer()
{
	lineDrawVertices.Reserve(DR_VERTEX_LINEBUFFER_SIZE);
	lineDrawOverlayVertices.Reserve(DR_VERTEX_LINEBUFFER_SIZE);

	solidDrawVertices.Reserve(DR_VERTEX_SOLIDBUFFER_SIZE);
	solidDrawOverlayVertices.Reserve(DR_VERTEX_SOLIDBUFFER_SIZE);

	FBufferDescriptor desc{};
	desc.bufferSize = DR_VERTEX_LINEBUFFER_SIZE;
	desc.data = nullptr;
	desc.dataStride = sizeof(FVertex);
	desc.flags = TH_BUFFER_FLAGS_CPU_WRITE;
	desc.type = TH_BUFFER_TYPE_VERTEX_BUFFER;

	lineMesh.vertexBuffer = gGHI->CreateBuffer(desc);
	lineMesh.numVertices = 0;
	lineMesh.topologyType = FMesh::TOPOLOGY_LINES;
	lineMesh.bSkinnedMesh = false;
	lineOverlayMesh.vertexBuffer = gGHI->CreateBuffer(desc);
	lineOverlayMesh.numVertices = 0;
	lineOverlayMesh.topologyType = FMesh::TOPOLOGY_LINES;
	lineOverlayMesh.bSkinnedMesh = false;

	desc.bufferSize = DR_VERTEX_SOLIDBUFFER_SIZE;

	solidMesh.vertexBuffer = gGHI->CreateBuffer(desc);
	solidMesh.numVertices = 0;
	solidMesh.topologyType = FMesh::TOPOLOGY_TRIANGLES;
	solidMesh.bSkinnedMesh = false;
	solidOverlayMesh.vertexBuffer = gGHI->CreateBuffer(desc);
	solidOverlayMesh.numVertices = 0;
	solidOverlayMesh.topologyType = FMesh::TOPOLOGY_TRIANGLES;
	solidOverlayMesh.bSkinnedMesh = false;

	cube = CAssetManager::GetAsset<CModelAsset>("models/Cube.thasset");
	sphere = CAssetManager::GetAsset<CModelAsset>("models/Sphere.thasset");

	matDebugLine = CreateObject<CMaterial>();
	matDebugLine->SetName("DebugDrawLine");
	matDebugLine->SetShader("Tools");
	matDebugLine->SetInt("vType", 4);

	if (cube)
		cube->Load(0);
	if (sphere)
		sphere->Load(0);
}

void CDebugRenderer::DrawLine(const FVector& begin, const FVector& end, const FColor& color, float time /*= 0.f*/, bool bOverlay /*= false*/)
{
	_Line(begin, end, color, nullptr, bOverlay);
}

void CDebugRenderer::DrawPlane(const FTransform& t, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	CMaterial* mat = CreateObject<CMaterial>();
	mat->SetName("DebugDrawPlane");
	mat->SetShader("Tools");
	mat->SetInt("vType", 4);
	mat->SetColor("vColorTint", col);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::PLANE,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawBox(const FTransform& t, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FDebugDrawCmd cmd{
		FDebugDrawCmd::BOX,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawCircle(const FVector& pos, const FVector& angle, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = pos;
	t.rotation = FQuaternion::LookRotation(angle, FVector::up);
	t.scale = radius;

	FDebugDrawCmd cmd{
		FDebugDrawCmd::CIRCLE,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawSphere(const FVector& pos, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = pos;
	t.scale = radius;

	FDebugDrawCmd cmd{
		FDebugDrawCmd::SPHERE,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawCylinder(const FVector& center, const FQuaternion& rot, float height, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = center;
	t.rotation = rot;

	FDebugDrawCmd cmd{
		FDebugDrawCmd::CYLINDER,
		drawType,
		t,
		col,
		FString(),
		height, radius,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawCapsule(const FVector& center, const FQuaternion& rot, float height, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = center;
	t.rotation = rot;

	FDebugDrawCmd cmd{
		FDebugDrawCmd::CAPSULE,
		drawType,
		t,
		col,
		FString(),
		height, radius,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawCone(const FVector& apex, const FQuaternion& rot, float height, float baseRadius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = apex;
	t.rotation = rot;

	FDebugDrawCmd cmd{
		FDebugDrawCmd::CONE,
		drawType,
		t,
		col,
		FString(),
		height, baseRadius,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawText(const FVector2& screenPos, const FString& text, const FColor& col /*= FColor()*/, float time /*= 0.f*/)
{
	FTransform t;
	t.position = screenPos;

	FDebugDrawCmd cmd{
		FDebugDrawCmd::TEXT,
		DebugDrawType_Overlay,
		t,
		col,
		text,
		0, 0,
		time + GetScene()->GetTime(),
		GetScene()
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawText3D(const FVector& pos, const FString& text, const FColor& col /*= FColor()*/, float time /*= 0.f*/, bool bOverlay /*= false*/)
{
	FTransform t;
	t.position = pos;

	FDebugDrawCmd cmd{
		FDebugDrawCmd::TEXT,
		bOverlay ? DebugDrawType_Overlay : DebugDrawType_None,
		t,
		col,
		text,
		0, 0,
		time,
		scene
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::SetScene(CRenderScene* scene)
{
	this->scene = scene;
}

void CDebugRenderer::Render()
{
	for (auto it = drawCalls.rbegin(); it != drawCalls.rend(); it++)
	{
		CRenderScene* scene = it->target;

		switch (it->type)
		{
		case FDebugDrawCmd::LINE:
			//_Line(it->transform, it->mat, scene, it->drawType & DebugDrawType_Overlay);
			break;
		case FDebugDrawCmd::PLANE:
		{
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& pos = it->transform.position;
				FVector& scale = it->transform.scale;
				FQuaternion& rot = it->transform.rotation;
				
				FVector corners[4] = {
					FVector(-0.5f, 0, -0.5f) * scale,
					FVector(0.5f, 0, -0.5f) * scale,
					FVector(0.5f, 0, 0.5f) * scale,
					FVector(-0.5f, 0, 0.5f) * scale
				};
				
				for (int i = 0; i < 4; i++)
				{
					_Line(pos + rot.Rotate(corners[i]), pos + rot.Rotate(corners[(i + 1) % 4]), it->color, scene, it->drawType & DebugDrawType_Overlay);
				}
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				_AddSolidPlane(it->transform, it->color, it->drawType & DebugDrawType_Overlay);
			}
		}
			break;
		case FDebugDrawCmd::BOX:
		{
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& pos = it->transform.position;
				FVector& scale = it->transform.scale;
				FQuaternion& rot = it->transform.rotation;
				_Line(rot.Rotate(pos + (FVector(-0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, -0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, 0.5f, -0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(-0.5f, -0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, 0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);

				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, -0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(-0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, -0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, 0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, 0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(-0.5f, 0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);

				_Line(rot.Rotate(pos + (FVector(0.5f, 0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, 0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, -0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, -0.5f, 0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, -0.5f, -0.5f) * scale)), it->color, scene, it->drawType & DebugDrawType_Overlay);
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				_AddSolidBox(it->transform, it->color, it->drawType & DebugDrawType_Overlay);
			}
		}
			break;
		case FDebugDrawCmd::SPHERE:
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& pos = it->transform.position;
				float radius = it->transform.scale.x;
				
				const int segments = 16;
				float segmentAngle = FMath::Pi() * 2.0f / segments;
				
				for (int lat = 1; lat < 3; lat++)
				{
					float latAngle = (FMath::Pi() / 4.0f) * lat;
					float latRadius = radius * FMath::Sin(latAngle);
					float latHeight = radius * FMath::Cos(latAngle);
					
					FVector prevPoint(latRadius * FMath::Cos(0), latHeight, latRadius * FMath::Sin(0));
					for (int lon = 1; lon <= segments; lon++)
					{
						FVector currPoint(latRadius * FMath::Cos(segmentAngle * lon), latHeight, latRadius * FMath::Sin(segmentAngle * lon));
						_Line(pos + prevPoint, pos + currPoint, it->color, scene, it->drawType & DebugDrawType_Overlay);
						prevPoint = currPoint;
					}
				}
				
				for (int lon = 0; lon < segments; lon++)
				{
					FVector prevPoint(radius * FMath::Cos(segmentAngle * lon), 0, radius * FMath::Sin(segmentAngle * lon));
					for (int lat = 1; lat <= 8; lat++)
					{
						float latAngle = (FMath::Pi() / 8.0f) * lat;
						FVector currPoint(radius * FMath::Sin(latAngle) * FMath::Cos(segmentAngle * lon), radius * FMath::Cos(latAngle), radius * FMath::Sin(latAngle) * FMath::Sin(segmentAngle * lon));
						_Line(pos + prevPoint, pos + currPoint, it->color, scene, it->drawType & DebugDrawType_Overlay);
						prevPoint = currPoint;
					}
				}
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				_AddSolidSphere(it->transform, it->color, it->drawType & DebugDrawType_Overlay);
			}
			break;
		case FDebugDrawCmd::CIRCLE:
		{
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& pos = it->transform.position;
				float radius = it->transform.scale.x;
				FQuaternion& rot = it->transform.rotation;
				
				const int segments = 32;
				float segmentAngle = FMath::Pi() * 2.0f / segments;
				
				FVector prevPoint(radius * FMath::Cos(0), 0, radius * FMath::Sin(0));
				for (int i = 1; i <= segments; i++)
				{
					FVector currPoint(radius * FMath::Cos(segmentAngle * i), 0, radius * FMath::Sin(segmentAngle * i));
					_Line(pos + rot.Rotate(prevPoint), pos + rot.Rotate(currPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
					prevPoint = currPoint;
				}
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				_AddSolidCircle(it->transform, it->color, it->drawType & DebugDrawType_Overlay);
			}
		}
			break;
		case FDebugDrawCmd::CYLINDER:
		{
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& pos = it->transform.position;
				float height = it->height;
				float radius = it->radius;
				FQuaternion& rot = it->transform.rotation;
				
				const int segments = 16;
				float segmentAngle = FMath::Pi() * 2.0f / segments;
				
				for (int circle = 0; circle < 2; circle++)
				{
					float yOffset = (circle == 0 ? height / 2.0f : -height / 2.0f);
					FVector prevPoint(radius * FMath::Cos(0), yOffset, radius * FMath::Sin(0));
					
					for (int i = 1; i <= segments; i++)
					{
						FVector currPoint(radius * FMath::Cos(segmentAngle * i), yOffset, radius * FMath::Sin(segmentAngle * i));
						_Line(pos + rot.Rotate(prevPoint), pos + rot.Rotate(currPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
						prevPoint = currPoint;
					}
				}
				
				for (int i = 0; i < segments; i += 4)
				{
					FVector topPoint(radius * FMath::Cos(segmentAngle * i), height / 2.0f, radius * FMath::Sin(segmentAngle * i));
					FVector bottomPoint(radius * FMath::Cos(segmentAngle * i), -height / 2.0f, radius * FMath::Sin(segmentAngle * i));
					_Line(pos + rot.Rotate(topPoint), pos + rot.Rotate(bottomPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
				}
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				_AddSolidCylinder(it->transform, it->height, it->radius, it->color, it->drawType & DebugDrawType_Overlay);
			}
		}
			break;
		case FDebugDrawCmd::CAPSULE:
		{
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& pos = it->transform.position;
				float height = it->height;
				float radius = it->radius;
				FQuaternion& rot = it->transform.rotation;
				
				const int segments = 16;
				float segmentAngle = FMath::Pi() * 2.0f / segments;
				
				float cylinderHeight = height - 2.0f * radius;
				
				for (int circle = 0; circle < 2; circle++)
				{
					float yOffset = (circle == 0 ? cylinderHeight / 2.0f : -cylinderHeight / 2.0f);
					FVector prevPoint(radius * FMath::Cos(0), yOffset, radius * FMath::Sin(0));
					
					for (int i = 1; i <= segments; i++)
					{
						FVector currPoint(radius * FMath::Cos(segmentAngle * i), yOffset, radius * FMath::Sin(segmentAngle * i));
						_Line(pos + rot.Rotate(prevPoint), pos + rot.Rotate(currPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
						prevPoint = currPoint;
					}
				}
				
				for (int i = 0; i < segments; i += 4)
				{
					FVector topPoint(radius * FMath::Cos(segmentAngle * i), cylinderHeight / 2.0f, radius * FMath::Sin(segmentAngle * i));
					FVector bottomPoint(radius * FMath::Cos(segmentAngle * i), -cylinderHeight / 2.0f, radius * FMath::Sin(segmentAngle * i));
					_Line(pos + rot.Rotate(topPoint), pos + rot.Rotate(bottomPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
				}
				
				for (int i = 0; i < segments; i += 4)
				{
					float angle = segmentAngle * i;
					FVector prevHemiPoint(radius * FMath::Cos(0) * FMath::Sin(angle), cylinderHeight / 2.0f + radius * FMath::Cos(0), radius * FMath::Sin(0) * FMath::Sin(angle));
					for (int lat = 1; lat <= 4; lat++)
					{
						float latAngle = (FMath::Pi() / 8.0f) * lat;
						FVector currHemiPoint(radius * FMath::Sin(latAngle) * FMath::Sin(angle), cylinderHeight / 2.0f + radius * FMath::Cos(latAngle), radius * FMath::Sin(latAngle) * FMath::Sin(angle));
						_Line(pos + rot.Rotate(prevHemiPoint), pos + rot.Rotate(currHemiPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
						prevHemiPoint = currHemiPoint;
					}
					
					prevHemiPoint = FVector(radius * FMath::Cos(0) * FMath::Sin(angle), -cylinderHeight / 2.0f + radius * FMath::Cos(0), radius * FMath::Sin(0) * FMath::Sin(angle));
					for (int lat = 1; lat <= 4; lat++)
					{
						float latAngle = (FMath::Pi() / 8.0f) * lat;
						FVector currHemiPoint(radius * FMath::Sin(latAngle) * FMath::Sin(angle), -cylinderHeight / 2.0f - radius * FMath::Cos(latAngle), radius * FMath::Sin(latAngle) * FMath::Sin(angle));
						_Line(pos + rot.Rotate(prevHemiPoint), pos + rot.Rotate(currHemiPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
						prevHemiPoint = currHemiPoint;
					}
				}
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				_AddSolidCapsule(it->transform, it->height, it->radius, it->color, it->drawType & DebugDrawType_Overlay);
			}
		}
			break;
		case FDebugDrawCmd::CONE:
		{
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& apex = it->transform.position;
				float height = it->height;
				float baseRadius = it->radius;
				FQuaternion& rot = it->transform.rotation;

				const int segments = 16;
				float segmentAngle = FMath::Pi() * 2.0f / segments;

				// Draw base circle
				FVector prevPoint(baseRadius * FMath::Cos(0), -height, baseRadius * FMath::Sin(0));
				for (int i = 1; i <= segments; i++)
				{
					FVector currPoint(baseRadius * FMath::Cos(segmentAngle * i), -height, baseRadius * FMath::Sin(segmentAngle * i));
					_Line(apex + rot.Rotate(prevPoint), apex + rot.Rotate(currPoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
					prevPoint = currPoint;
				}

				// Draw lines from apex to base
				for (int i = 0; i < segments; i += 4)
				{
					FVector basePoint(baseRadius * FMath::Cos(segmentAngle * i), -height, baseRadius * FMath::Sin(segmentAngle * i));
					_Line(apex, apex + rot.Rotate(basePoint), it->color, scene, it->drawType & DebugDrawType_Overlay);
				}
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				_AddSolidCone(it->transform, it->height, it->radius, it->color, it->drawType & DebugDrawType_Overlay);
			}
		}
			break;
		case FDebugDrawCmd::TEXT:
		{
		}
			break;
		}

		if (it->time < scene->GetTime())
		{
			drawCalls.Erase(it);
			continue;
		}
	}

	lineMesh.vertexBuffer->Update(DR_VERTEX_LINEBUFFER_SIZE, lineDrawVertices.Data());
	lineMesh.numVertices = lineDrawVertices.Size();
	lineDrawVertices.Clear();

	lineOverlayMesh.vertexBuffer->Update(DR_VERTEX_LINEBUFFER_SIZE, lineDrawOverlayVertices.Data());
	lineOverlayMesh.numVertices = lineDrawOverlayVertices.Size();
	lineDrawOverlayVertices.Clear();

	solidMesh.vertexBuffer->Update(DR_VERTEX_SOLIDBUFFER_SIZE, solidDrawVertices.Data());
	solidMesh.numVertices = solidDrawVertices.Size();
	solidDrawVertices.Clear();

	solidOverlayMesh.vertexBuffer->Update(DR_VERTEX_SOLIDBUFFER_SIZE, solidDrawOverlayVertices.Data());
	solidOverlayMesh.numVertices = solidDrawOverlayVertices.Size();
	solidDrawOverlayVertices.Clear();

	FDrawMeshCmd cmd{};
	cmd.mesh = &lineMesh;
	cmd.material = matDebugLine;
	cmd.drawType = MESH_DRAW_PRIMITIVE_LINES;
	cmd.transform = FMatrix(1.f);

	scene->PushCommand(FRenderCommand(cmd, R_DEBUG_PASS));

	cmd.mesh = &lineOverlayMesh;
	scene->PushCommand(FRenderCommand(cmd, R_DEBUG_OVERLAY_PASS));

	cmd.mesh = &solidMesh;
	cmd.drawType = MESH_DRAW_NONE;
	scene->PushCommand(FRenderCommand(cmd, R_DEBUG_PASS));

	cmd.mesh = &solidOverlayMesh;
	scene->PushCommand(FRenderCommand(cmd, R_DEBUG_OVERLAY_PASS));
}

CRenderScene* CDebugRenderer::GetScene()
{
	return scene ? scene : gWorld->GetRenderScene();
}

void CDebugRenderer::_Line(const FVector& begin, const FVector& end, const FColor& col, CRenderScene* scene, bool bOverlay)
{
	FVertex v1{};
	FVertex v2{};

	v1.position = begin;
	v2.position = end;
	v1.color = { col.r, col.g, col.b };
	v2.color = v1.color;
	v1.uv1[0] = col.a;
	v2.uv1[0] = col.a;

	if (bOverlay)
	{
		if (lineDrawOverlayVertices.Size() + 2 >= DR_VERTEX_LINEBUFFER_SIZE)
			return;

		lineDrawOverlayVertices.Add(v1);
		lineDrawOverlayVertices.Add(v2);
	}
	else
	{
		if (lineDrawVertices.Size() + 2 >= DR_VERTEX_LINEBUFFER_SIZE)
			return;

		lineDrawVertices.Add(v1);
		lineDrawVertices.Add(v2);
	}
}

void CDebugRenderer::_AddSolidVertex(const FVector& pos, const FColor& col, bool bOverlay)
{
	FVertex v{};
	v.position = pos;
	v.color = { col.r, col.g, col.b };
	v.uv1[0] = col.a;

	if (bOverlay)
	{
		if (solidDrawOverlayVertices.Size() + 1 >= DR_VERTEX_SOLIDBUFFER_SIZE)
			return;
		solidDrawOverlayVertices.Add(v);
	}
	else
	{
		if (solidDrawVertices.Size() + 1 >= DR_VERTEX_SOLIDBUFFER_SIZE)
			return;
		solidDrawVertices.Add(v);
	}
}

void CDebugRenderer::_AddSolidTriangle(const FVector& p0, const FVector& p1, const FVector& p2, const FColor& col, bool bOverlay)
{
	_AddSolidVertex(p0, col, bOverlay);
	_AddSolidVertex(p1, col, bOverlay);
	_AddSolidVertex(p2, col, bOverlay);
}

void CDebugRenderer::_AddSolidBox(const FTransform& t, const FColor& col, bool bOverlay)
{
	FVector pos = t.position;
	FVector scale = t.scale;
	FQuaternion rot = t.rotation;

	// Define box corners in local space
	FVector corners[8] = {
		FVector(-0.5f, -0.5f, -0.5f) * scale,
		FVector(0.5f, -0.5f, -0.5f) * scale,
		FVector(0.5f, 0.5f, -0.5f) * scale,
		FVector(-0.5f, 0.5f, -0.5f) * scale,
		FVector(-0.5f, -0.5f, 0.5f) * scale,
		FVector(0.5f, -0.5f, 0.5f) * scale,
		FVector(0.5f, 0.5f, 0.5f) * scale,
		FVector(-0.5f, 0.5f, 0.5f) * scale
	};

	for (int i = 0; i < 8; i++)
		corners[i] = pos + rot.Rotate(corners[i]);

	_AddSolidTriangle(corners[0], corners[1], corners[2], col, bOverlay);
	_AddSolidTriangle(corners[0], corners[2], corners[3], col, bOverlay);

	_AddSolidTriangle(corners[5], corners[4], corners[7], col, bOverlay);
	_AddSolidTriangle(corners[5], corners[7], corners[6], col, bOverlay);

	_AddSolidTriangle(corners[4], corners[0], corners[3], col, bOverlay);
	_AddSolidTriangle(corners[4], corners[3], corners[7], col, bOverlay);

	_AddSolidTriangle(corners[1], corners[5], corners[6], col, bOverlay);
	_AddSolidTriangle(corners[1], corners[6], corners[2], col, bOverlay);

	_AddSolidTriangle(corners[3], corners[2], corners[6], col, bOverlay);
	_AddSolidTriangle(corners[3], corners[6], corners[7], col, bOverlay);

	_AddSolidTriangle(corners[4], corners[5], corners[1], col, bOverlay);
	_AddSolidTriangle(corners[4], corners[1], corners[0], col, bOverlay);
}

void CDebugRenderer::_AddSolidPlane(const FTransform& t, const FColor& col, bool bOverlay)
{
	FVector pos = t.position;
	FVector scale = t.scale;
	FQuaternion rot = t.rotation;

	FVector corners[4] = {
		FVector(-0.5f, 0, -0.5f) * scale,
		FVector(0.5f, 0, -0.5f) * scale,
		FVector(0.5f, 0, 0.5f) * scale,
		FVector(-0.5f, 0, 0.5f) * scale
	};

	for (int i = 0; i < 4; i++)
	{
		corners[i] = pos + rot.Rotate(corners[i]);
	}

	_AddSolidTriangle(corners[0], corners[1], corners[2], col, bOverlay);
	_AddSolidTriangle(corners[0], corners[2], corners[3], col, bOverlay);
}

void CDebugRenderer::_AddSolidCircle(const FTransform& t, const FColor& col, bool bOverlay)
{
	FVector pos = t.position;
	float radius = t.scale.x;
	FQuaternion rot = t.rotation;

	const int segments = 32;
	float segmentAngle = FMath::Pi() * 2.0f / segments;

	for (int i = 0; i < segments; i++)
	{
		float angle0 = segmentAngle * i;
		float angle1 = segmentAngle * (i + 1);

		FVector p0(radius * FMath::Cos(angle0), 0, radius * FMath::Sin(angle0));
		FVector p1(radius * FMath::Cos(angle1), 0, radius * FMath::Sin(angle1));

		p0 = pos + rot.Rotate(p0);
		p1 = pos + rot.Rotate(p1);

		_AddSolidTriangle(pos, p0, p1, col, bOverlay);
	}
}

void CDebugRenderer::_AddSolidSphere(const FTransform& t, const FColor& col, bool bOverlay)
{
	FVector pos = t.position;
	float radius = t.scale.x;

	const int lats = 8;
	const int lons = 16;

	for (int lat = 0; lat < lats; lat++)
	{
		float lat0 = FMath::Pi() * (float)(lat) / lats;
		float lat1 = FMath::Pi() * (float)(lat + 1) / lats;

		float y0 = radius * FMath::Cos(lat0);
		float y1 = radius * FMath::Cos(lat1);

		float r0 = radius * FMath::Sin(lat0);
		float r1 = radius * FMath::Sin(lat1);

		for (int lon = 0; lon < lons; lon++)
		{
			float lon0 = 2.0f * FMath::Pi() * (float)(lon) / lons;
			float lon1 = 2.0f * FMath::Pi() * (float)(lon + 1) / lons;

			FVector p0(r0 * FMath::Cos(lon0), y0, r0 * FMath::Sin(lon0));
			FVector p1(r0 * FMath::Cos(lon1), y0, r0 * FMath::Sin(lon1));
			FVector p2(r1 * FMath::Cos(lon1), y1, r1 * FMath::Sin(lon1));
			FVector p3(r1 * FMath::Cos(lon0), y1, r1 * FMath::Sin(lon0));

			_AddSolidTriangle(pos + p0, pos + p1, pos + p2, col, bOverlay);
			_AddSolidTriangle(pos + p0, pos + p2, pos + p3, col, bOverlay);
		}
	}
}

void CDebugRenderer::_AddSolidCylinder(const FTransform& t, float height, float radius, const FColor& col, bool bOverlay)
{
	FVector pos = t.position;
	FQuaternion rot = t.rotation;

	const int segments = 16;
	float segmentAngle = FMath::Pi() * 2.0f / segments;
	float halfHeight = height / 2.0f;

	// Top and bottom circle vertices
	TArray<FVector> topVerts, bottomVerts;

	for (int i = 0; i < segments; i++)
	{
		float angle = segmentAngle * i;
		float x = radius * FMath::Cos(angle);
		float z = radius * FMath::Sin(angle);

		FVector topVert = pos + rot.Rotate(FVector(x, halfHeight, z));
		FVector bottomVert = pos + rot.Rotate(FVector(x, -halfHeight, z));

		topVerts.Add(topVert);
		bottomVerts.Add(bottomVert);
	}

	// Top face
	for (int i = 0; i < segments; i++)
	{
		int next = (i + 1) % segments;
		_AddSolidTriangle(pos + rot.Rotate(FVector(0, halfHeight, 0)), topVerts[next], topVerts[i], col, bOverlay);
	}

	// Bottom face
	for (int i = 0; i < segments; i++)
	{
		int next = (i + 1) % segments;
		_AddSolidTriangle(pos + rot.Rotate(FVector(0, -halfHeight, 0)), bottomVerts[i], bottomVerts[next], col, bOverlay);
	}

	// Side faces
	for (int i = 0; i < segments; i++)
	{
		int next = (i + 1) % segments;
		_AddSolidTriangle(topVerts[i], bottomVerts[next], bottomVerts[i], col, bOverlay);
		_AddSolidTriangle(topVerts[i], topVerts[next], bottomVerts[next], col, bOverlay);
	}
}

void CDebugRenderer::_AddSolidCapsule(const FTransform& t, float height, float radius, const FColor& col, bool bOverlay)
{
	FVector pos = t.position;
	FQuaternion rot = t.rotation;

	float cylinderHeight = height - 2.0f * radius;
	float halfCylHeight = cylinderHeight / 2.0f;

	const int segments = 16;
	const int hemisegments = 8;
	float segmentAngle = FMath::Pi() * 2.0f / segments;

	// Top and bottom cylinder circle vertices
	TArray<FVector> topVerts, bottomVerts;

	for (int i = 0; i < segments; i++)
	{
		float angle = segmentAngle * i;
		float x = radius * FMath::Cos(angle);
		float z = radius * FMath::Sin(angle);

		FVector topVert = pos + rot.Rotate(FVector(x, halfCylHeight, z));
		FVector bottomVert = pos + rot.Rotate(FVector(x, -halfCylHeight, z));

		topVerts.Add(topVert);
		bottomVerts.Add(bottomVert);
	}

	// Cylinder side faces
	for (int i = 0; i < segments; i++)
	{
		int next = (i + 1) % segments;
		_AddSolidTriangle(topVerts[i], bottomVerts[i], bottomVerts[next], col, bOverlay);
		_AddSolidTriangle(topVerts[i], bottomVerts[next], topVerts[next], col, bOverlay);
	}

	// Top hemisphere
	for (int lat = 0; lat < hemisegments; lat++)
	{
		float lat0 = FMath::Pi() / 2.0f * (float)(lat) / hemisegments;
		float lat1 = FMath::Pi() / 2.0f * (float)(lat + 1) / hemisegments;

		float y0 = radius * FMath::Sin(lat0);
		float y1 = radius * FMath::Sin(lat1);

		float r0 = radius * FMath::Cos(lat0);
		float r1 = radius * FMath::Cos(lat1);

		for (int lon = 0; lon < segments; lon++)
		{
			float lon0 = segmentAngle * lon;
			float lon1 = segmentAngle * (lon + 1);

			FVector p0(r0 * FMath::Cos(lon0), halfCylHeight + y0, r0 * FMath::Sin(lon0));
			FVector p1(r0 * FMath::Cos(lon1), halfCylHeight + y0, r0 * FMath::Sin(lon1));
			FVector p2(r1 * FMath::Cos(lon1), halfCylHeight + y1, r1 * FMath::Sin(lon1));
			FVector p3(r1 * FMath::Cos(lon0), halfCylHeight + y1, r1 * FMath::Sin(lon0));

			_AddSolidTriangle(pos + rot.Rotate(p0), pos + rot.Rotate(p1), pos + rot.Rotate(p2), col, bOverlay);
			_AddSolidTriangle(pos + rot.Rotate(p0), pos + rot.Rotate(p2), pos + rot.Rotate(p3), col, bOverlay);
		}
	}

	// Bottom hemisphere
	for (int lat = 0; lat < hemisegments; lat++)
	{
		float lat0 = FMath::Pi() / 2.0f * (float)(lat) / hemisegments;
		float lat1 = FMath::Pi() / 2.0f * (float)(lat + 1) / hemisegments;

		float y0 = -radius * FMath::Sin(lat0);
		float y1 = -radius * FMath::Sin(lat1);

		float r0 = radius * FMath::Cos(lat0);
		float r1 = radius * FMath::Cos(lat1);

		for (int lon = 0; lon < segments; lon++)
		{
			float lon0 = segmentAngle * lon;
			float lon1 = segmentAngle * (lon + 1);

			FVector p0(r0 * FMath::Cos(lon0), -halfCylHeight + y0, r0 * FMath::Sin(lon0));
			FVector p1(r0 * FMath::Cos(lon1), -halfCylHeight + y0, r0 * FMath::Sin(lon1));
			FVector p2(r1 * FMath::Cos(lon1), -halfCylHeight + y1, r1 * FMath::Sin(lon1));
			FVector p3(r1 * FMath::Cos(lon0), -halfCylHeight + y1, r1 * FMath::Sin(lon0));

			_AddSolidTriangle(pos + rot.Rotate(p0), pos + rot.Rotate(p2), pos + rot.Rotate(p1), col, bOverlay);
			_AddSolidTriangle(pos + rot.Rotate(p0), pos + rot.Rotate(p3), pos + rot.Rotate(p2), col, bOverlay);
		}
	}
}

void CDebugRenderer::_AddSolidCone(const FTransform& t, float height, float baseRadius, const FColor& col, bool bOverlay)
{
	FVector apex = t.position;
	FQuaternion rot = t.rotation;

	const int segments = 16;
	float segmentAngle = FMath::Pi() * 2.0f / segments;

	TArray<FVector> baseVerts;

	for (int i = 0; i < segments; i++)
	{
		float angle = segmentAngle * i;
		float x = baseRadius * FMath::Cos(angle);
		float z = baseRadius * FMath::Sin(angle);

		FVector baseVert = apex + rot.Rotate(FVector(x, -height, z));
		baseVerts.Add(baseVert);
	}

	FVector baseCenter = apex + rot.Rotate(FVector(0, -height, 0));

	// Base face
	for (int i = 0; i < segments; i++)
	{
		int next = (i + 1) % segments;
		_AddSolidTriangle(baseCenter, baseVerts[i], baseVerts[next], col, bOverlay);
	}

	// Side faces
	for (int i = 0; i < segments; i++)
	{
		int next = (i + 1) % segments;
		_AddSolidTriangle(apex, baseVerts[next], baseVerts[i], col, bOverlay);
	}
}
