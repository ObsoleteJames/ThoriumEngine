
#include "DebugRenderer.h"
#include "Resources/Material.h"
#include "Resources/ModelAsset.h"
#include "Renderer.h"
#include "RenderScene.h"
#include "Game/World.h"

CDebugRenderer* gDebugRenderer;

CDebugRenderer::CDebugRenderer()
{
	lineMesh.numVertices = 2;
	lineMesh.vertexBuffer = gRenderer->CreateVertexBuffer({ { FVector::zero }, { -FVector::forward } });
	lineMesh.topologyType = FMesh::TOPOLOGY_LINES;

	cube = CResourceManager::GetResource<CModelAsset>("models/Cube.thmdl");
	sphere = CResourceManager::GetResource<CModelAsset>("models/Sphere.thmdl");
}

void CDebugRenderer::DrawLine(const FVector& begin, const FVector& end, const FColor& color, float time /*= 0.f*/, bool bOverlay /*= false*/)
{
	FTransform t;
	t.position = begin;
	FVector dir = (end - begin).Normalize();
	t.rotation = FQuaternion::LookRotation(dir, dir == FVector::up ? FVector::forward : FVector::up);
	t.scale = (end - begin).Magnitude();

	CMaterial* mat = CreateObject<CMaterial>();
	mat->SetName("DebugDrawLine");
	mat->SetShader("Tools");
	mat->SetInt("vType", 4);
	mat->SetColor("vColorTint", color);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::LINE,
		bOverlay ? DebugDrawType_Overlay : DebugDrawType_None,
		t,
		color,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene(),
		mat
	};
	drawCalls.Add(cmd);
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
		GetScene(),
		mat
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawBox(const FTransform& t, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	CMaterial* mat = CreateObject<CMaterial>();
	mat->SetName("DebugDrawBox");
	mat->SetShader("Tools");
	mat->SetInt("vType", 4);
	mat->SetColor("vColorTint", col);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::BOX,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene(),
		mat
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawCircle(const FVector& pos, const FVector& angle, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = pos;
	t.rotation = FQuaternion::LookRotation(angle, FVector::up);
	t.scale = radius;

	CMaterial* mat = CreateObject<CMaterial>();
	mat->SetName("DebugDrawCircle");
	mat->SetShader("Tools");
	mat->SetInt("vType", 4);
	mat->SetColor("vColorTint", col);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::CIRCLE,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene(),
		mat
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawSphere(const FVector& pos, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = pos;
	t.scale = radius;

	CMaterial* mat = CreateObject<CMaterial>();
	mat->SetName("DebugDrawSphere");
	mat->SetShader("Tools");
	mat->SetInt("vType", 4);
	mat->SetColor("vColorTint", col);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::SPHERE,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene(),
		mat
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawCylinder(const FVector& center, const FQuaternion& rot, float height, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = center;
	t.rotation = rot;

	CMaterial* mat = CreateObject<CMaterial>();
	mat->SetName("DebugDrawCylinder");
	mat->SetShader("Tools");
	mat->SetInt("vType", 4);
	mat->SetColor("vColorTint", col);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::CYLINDER,
		drawType,
		t,
		col,
		FString(),
		height, radius,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene(),
		mat
	};
	drawCalls.Add(cmd);
}

void CDebugRenderer::DrawCapsule(const FVector& center, const FQuaternion& rot, float height, float radius, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FTransform t;
	t.position = center;
	t.rotation = rot;

	CMaterial* mat = CreateObject<CMaterial>();
	mat->SetName("DebugDrawCapsule");
	mat->SetShader("Tools");
	mat->SetInt("vType", 4);
	mat->SetColor("vColorTint", col);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::CAPSULE,
		drawType,
		t,
		col,
		FString(),
		height, radius,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene(),
		mat
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
			_Line(it->transform, it->mat, scene, it->drawType & DebugDrawType_Overlay);
			break;
		case FDebugDrawCmd::BOX:
		{
			if (it->drawType & DebugDrawType_Wireframe)
			{
				FVector& pos = it->transform.position;
				FVector& scale = it->transform.scale;
				FQuaternion& rot = it->transform.rotation;
				_Line(rot.Rotate(pos + (FVector(-0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, -0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, 0.5f, -0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(-0.5f, -0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, 0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);

				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, -0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(-0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, -0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, 0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(0.5f, 0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(-0.5f, 0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);

				_Line(rot.Rotate(pos + (FVector(0.5f, 0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, 0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, 0.5f, -0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, 0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, -0.5f, 0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
				_Line(rot.Rotate(pos + (FVector(0.5f, -0.5f, -0.5f) * scale)), rot.Rotate(pos + (FVector(-0.5f, -0.5f, -0.5f) * scale)), it->mat, scene, it->drawType & DebugDrawType_Overlay);
			}
			if (it->drawType & DebugDrawType_Solid)
			{
				FDrawMeshCmd cmd{};
				cmd.mesh = (FMesh*)&cube->GetMeshes()[0];
				cmd.material = it->mat;
				cmd.transform = it->transform.ToMatrix();

				scene->PushCommand(FRenderCommand(cmd, it->drawType & DebugDrawType_Overlay ? R_DEBUG_OVERLAY_PASS : R_DEBUG_PASS));
			}
		}
			break;
		case FDebugDrawCmd::SPHERE:
			if (it->drawType & DebugDrawType_Wireframe)
			{

			}
			if (it->drawType & DebugDrawType_Solid)
			{
				FDrawMeshCmd cmd{};
				cmd.mesh = (FMesh*)&sphere->GetMeshes()[0];
				cmd.material = it->mat;
				cmd.transform = it->transform.ToMatrix();

				scene->PushCommand(FRenderCommand(cmd, it->drawType & DebugDrawType_Overlay ? R_DEBUG_OVERLAY_PASS : R_DEBUG_PASS));
			}
			break;
		}

		if (it->time < scene->GetTime())
		{
			drawCalls.Erase(it);
			continue;
		}
	}
}

CRenderScene* CDebugRenderer::GetScene()
{
	return scene ? scene : gWorld->GetRenderScene();
}

void CDebugRenderer::_Line(const FVector& begin, const FVector& end, CMaterial* mat, CRenderScene* scene, bool bOverlay)
{
	FDrawMeshCmd cmd{};
	cmd.mesh = &lineMesh;
	cmd.material = mat;
	cmd.drawType = MESH_DRAW_PRIMITIVE_LINES;
	cmd.transform = FMatrix(1.f).Translate(begin).Scale((end - begin).Magnitude()) * FQuaternion::LookRotation((end - begin).Normalize(), FVector::up);

	scene->PushCommand(FRenderCommand(cmd, bOverlay ? R_DEBUG_OVERLAY_PASS : R_DEBUG_PASS));
}

void CDebugRenderer::_Line(const FTransform& t, CMaterial* mat, CRenderScene* scene, bool bOverlay)
{
	FDrawMeshCmd cmd{};
	cmd.mesh = &lineMesh;
	cmd.material = mat;
	cmd.drawType = MESH_DRAW_PRIMITIVE_LINES;
	cmd.transform = t.ToMatrix();

	scene->PushCommand(FRenderCommand(cmd, bOverlay ? R_DEBUG_OVERLAY_PASS : R_DEBUG_PASS));
}
