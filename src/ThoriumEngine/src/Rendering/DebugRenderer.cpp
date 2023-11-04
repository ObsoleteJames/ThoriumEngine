
#include "DebugRenderer.h"


void CDebugRenderer::DrawLine(const FVector& begin, const FVector& end, const FColor& color, float time /*= 0.f*/, bool bOverlay /*= false*/)
{
	FTransform t;
	t.position = begin;
	t.rotation = FQuaternion::LookRotation((end - begin).Normalize(), FVector::up);
	t.scale = (end - begin).Magnitude();

	FDebugDrawCmd cmd{
		FDebugDrawCmd::LINE,
		DebugDrawType_None,
		t,
		color,
		FString(),
		0, 0,
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
}

void CDebugRenderer::DrawPlane(const FTransform& t, const FColor& col, EDebugDrawType drawType, float time /*= 0.f*/)
{
	FDebugDrawCmd cmd{
		FDebugDrawCmd::PLANE,
		drawType,
		t,
		col,
		FString(),
		0, 0,
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
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
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
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
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
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
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
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
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
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
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
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
		time,
		scene
	};
	drawCalls.Add(cmd);
	scene = nullptr;
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
	scene = nullptr;
}

void CDebugRenderer::SetNextRenderScene(CRenderScene* scene)
{
	this->scene = scene;
}

void CDebugRenderer::Render()
{

}
