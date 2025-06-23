
#include "DebugRenderer.h"
#include "Assets/Material.h"
#include "Assets/ModelAsset.h"
#include "Renderer.h"
#include "GraphicsInterface.h"
#include "RenderScene.h"
#include "Game/World.h"

CDebugRenderer* gDebugRenderer;

#define DR_VERTEX_LINEBUFFER_SIZE sizeof(FVertex) * 512

CDebugRenderer::CDebugRenderer()
{
	lineMesh.numVertices = 0;
	lineMesh.vertexBuffer = gGHI->CreateVertexBuffer(DR_VERTEX_LINEBUFFER_SIZE);
	lineMesh.topologyType = FMesh::TOPOLOGY_LINES;
	lineOverlayMesh.numVertices = 0;
	lineOverlayMesh.vertexBuffer = gGHI->CreateVertexBuffer(DR_VERTEX_LINEBUFFER_SIZE);
	lineOverlayMesh.topologyType = FMesh::TOPOLOGY_LINES;

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
	//FVertex v1{};
	//FVertex v2{};

	//v1.position = begin;
	//v2.position = end;
	//v1.color = { color.r, color.g, color.b };
	//v2.color = v1.color;

	//lineDrawVertices.Add(v1);
	//lineDrawVertices.Add(v2);

	FTransform t;
	t.position = begin;
	t.scale = end; // use scale as end position

	//CMaterial* mat = CreateObject<CMaterial>();
	//mat->SetName("DebugDrawLine");
	//mat->SetShader("Tools");
	//mat->SetInt("vType", 4);
	//mat->SetColor("vColorTint", color);

	FDebugDrawCmd cmd{
		FDebugDrawCmd::LINE,
		bOverlay ? DebugDrawType_Overlay : DebugDrawType_None,
		t,
		color,
		FString(),
		0, 0,
		time != 0.f ? time + GetScene()->GetTime() : 0.f,
		GetScene(),
		matDebugLine
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

	//lineMesh.vertexBuffer->Update(lineDrawVertices.Size(), lineDrawVertices.Data());
	//lineMesh.numVertices = lineDrawVertices.Size();
	//lineDrawVertices.Clear();

	//lineOverlayMesh.vertexBuffer->Update(lineDrawOverlayVertices.Size(), lineDrawOverlayVertices.Data());
	//lineOverlayMesh.numVertices = lineDrawVertices.Size();
	//lineDrawOverlayVertices.Clear();

	//FDrawMeshCmd cmd{};
	//cmd.mesh = &lineMesh;
	//cmd.material = matDebugLine;
	//cmd.drawType = MESH_DRAW_PRIMITIVE_LINES;
	//cmd.transform = FMatrix(1.f);

	//scene->PushCommand(FRenderCommand(cmd, R_DEBUG_PASS));

	//cmd.mesh = &lineOverlayMesh;
	//scene->PushCommand(FRenderCommand(cmd, R_DEBUG_OVERLAY_PASS));
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

	/*FDrawMeshCmd cmd{};
	cmd.mesh = &lineMesh;
	cmd.material = mat;
	cmd.drawType = MESH_DRAW_PRIMITIVE_LINES;
	cmd.transform = FMatrix(1.f).Translate(begin).Scale((end - begin).Magnitude()) * FQuaternion::LookRotation((end - begin).Normalize(), FVector::up);

	scene->PushCommand(FRenderCommand(cmd, bOverlay ? R_DEBUG_OVERLAY_PASS : R_DEBUG_PASS));*/
}

void CDebugRenderer::_Line(const FTransform& t, CMaterial* mat, CRenderScene* scene, bool bOverlay)
{
	/*FDrawMeshCmd cmd{};
	cmd.mesh = &lineMesh;
	cmd.material = mat;
	cmd.drawType = MESH_DRAW_PRIMITIVE_LINES;
	cmd.transform = t.ToMatrix();

	scene->PushCommand(FRenderCommand(cmd, bOverlay ? R_DEBUG_OVERLAY_PASS : R_DEBUG_PASS));*/
}
