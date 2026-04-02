#pragma once

#include "EngineCore.h"
#include "Math/Vectors.h"
#include "Math/Color.h"
#include "Math/Transform.h"
#include "Assets/Mesh.h"

class CMaterial;
class CRenderScene;
class CDebugRenderer;
class CModelAsset;

//extern ENGINE_API CDebugRenderer* gDebugRenderer;

#define gDebugRenderer (gWorld->GetRenderScene()->DebugRenderer())

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
	void DrawCone(const FVector& apex, const FQuaternion& rot, float height, float baseRadius, const FColor& col, EDebugDrawType drawType, float time = 0.f);
	
	inline void DrawBounds(const FBounds& bounds, const FColor& col, bool bOverlay = false, float time = 0.f) { DrawBox(FTransform(bounds.position, FQuaternion(), bounds.Size()), col, bOverlay ? (DebugDrawType_Overlay | DebugDrawType_Wireframe) : DebugDrawType_Wireframe, time); }
	inline void DrawBounds(const FBounds& bounds, const FQuaternion& rot, const FColor& col, bool bOverlay = false, float time = 0.f) { DrawBox(FTransform(bounds.position, rot, bounds.Size()), col, bOverlay ? (DebugDrawType_Overlay | DebugDrawType_Wireframe) : DebugDrawType_Wireframe, time); }

	void DrawText(const FVector2& screenPos, const FString& text, const FColor& col = FColor(), float time = 0.f);
	void DrawText3D(const FVector& pos, const FString& text, const FColor& col = FColor(), float time = 0.f, bool bOverlay = false);

	// Set RenderScene for the next draw call, by default the gWorld's RenderScene is used.
	void SetScene(CRenderScene* scene);

	void Render();

private:
	CRenderScene* GetScene();

	void _Line(const FVector& begin, const FVector& end, const FColor& col, CRenderScene* scene, bool bOverlay);

	void _AddSolidVertex(const FVector& pos, const FColor& col, bool bOverlay);
	void _AddSolidTriangle(const FVector& p0, const FVector& p1, const FVector& p2, const FColor& col, bool bOverlay);
	void _AddSolidBox(const FTransform& t, const FColor& col, bool bOverlay);
	void _AddSolidPlane(const FTransform& t, const FColor& col, bool bOverlay);
	void _AddSolidCircle(const FTransform& t, const FColor& col, bool bOverlay);
	void _AddSolidSphere(const FTransform& t, const FColor& col, bool bOverlay);
	void _AddSolidCylinder(const FTransform& t, float height, float radius, const FColor& col, bool bOverlay);
	void _AddSolidCapsule(const FTransform& t, float height, float radius, const FColor& col, bool bOverlay);
	void _AddSolidCone(const FTransform& t, float height, float baseRadius, const FColor& col, bool bOverlay);

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
			CONE,
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
	};

	FMesh lineMesh;
	FMesh lineOverlayMesh;
	FMesh solidMesh;
	FMesh solidOverlayMesh;
	FMesh textMesh;
	TObjectPtr<CModelAsset> cube;
	TObjectPtr<CModelAsset> sphere;

	TArray<FVertex> lineDrawVertices;
	TArray<FVertex> lineDrawOverlayVertices;

	TArray<FVertex> solidDrawVertices;
	TArray<FVertex> solidDrawOverlayVertices;

	TArray<FVertex> textDrawVertices;

	TObjectPtr<CMaterial> matDebugLine;

	CRenderScene* scene = nullptr;
	TArray<FDebugDrawCmd> drawCalls;
};
