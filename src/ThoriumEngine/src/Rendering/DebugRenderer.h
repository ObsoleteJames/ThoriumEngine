#pragma once

#include "EngineCore.h"
#include "Math/Vectors.h"
#include "Math/Color.h"
#include "Math/Transform.h"
#include "Resources/Mesh.h"

class CMaterial;
class CRenderScene;
class CDebugRenderer;
class CModelAsset;

extern ENGINE_API CDebugRenderer* gDebugRenderer;

enum EDebugDrawType_
{
	DebugDrawType_None = 0,
	DebugDrawType_Solid = 1 << 0,
	DebugDrawType_Wireframe = 1 << 1,
	DebugDrawType_Overlay = 1 << 2
};
typedef int EDebugDrawType;

class ENGINE_API CDebugRenderer
{
	friend class IRenderer;

public:
	CDebugRenderer();

	void DrawLine(const FVector& begin, const FVector& end, const FColor& color, float time = 0.f, bool bOverlay = false);
	void DrawPlane(const FTransform& t, const FColor& col, EDebugDrawType drawType, float time = 0.f);
	void DrawBox(const FTransform& t, const FColor& col, EDebugDrawType drawType, float time = 0.f);
	void DrawCircle(const FVector& pos, const FVector& dir, float radius, const FColor& col, EDebugDrawType drawType, float time = 0.f);
	void DrawSphere(const FVector& pos, float radius, const FColor& col, EDebugDrawType drawType, float time = 0.f);
	void DrawCylinder(const FVector& center, const FQuaternion& rot, float height, float radius, const FColor& col, EDebugDrawType drawType, float time = 0.f);
	void DrawCapsule(const FVector& center, const FQuaternion& rot, float height, float radius, const FColor& col, EDebugDrawType drawType, float time = 0.f);
	
	void DrawText(const FVector2& screenPos, const FString& text, const FColor& col = FColor(), float time = 0.f);
	void DrawText3D(const FVector& pos, const FString& text, const FColor& col = FColor(), float time = 0.f, bool bOverlay = false);

	// Set RenderScene for the next draw call, by default the gWorld's RenderScene is used.
	void SetNextRenderScene(CRenderScene* scene);

private:
	void Render();

	CRenderScene* GetScene();

	void _Line(const FVector& begin, const FVector& end, CMaterial* mat, CRenderScene* scene, bool bOverlay);
	void _Line(const FTransform& t, CMaterial* mat, CRenderScene* scene, bool bOverlay);

private:
	struct FDebugDrawCmd
	{
		enum EType
		{
			LINE,
			PLANE,
			BOX,
			CIRCLE,
			SPHERE,
			CYLINDER,
			CAPSULE,
			TEXT,
			TEXT_3D
		};

		EType type;
		EDebugDrawType drawType;

		FTransform transform;
		FColor color;
		FString text;
		float height;
		float radius;

		// how long this should be drawn for.
		float time;

		CRenderScene* target;
		TObjectPtr<CMaterial> mat;
	};

	FMesh lineMesh;
	TObjectPtr<CModelAsset> cube;
	TObjectPtr<CModelAsset> sphere;

	CRenderScene* scene = nullptr;
	TArray<FDebugDrawCmd> drawCalls;
};
