#pragma once

#include "EngineCore.h"
#include "Math/Vectors.h"
#include "Math/Color.h"
#include "Math/Transform.h"

class CMaterial;

class ENGINE_API CDebugRenderer
{
public:
	void DrawLine(const FVector& begin, const FVector& end, const FColor& color, bool bOverlay = false);
	void DrawPlane(const FTransform& transform, CMaterial* mat, bool bOverlay = false);

	void DrawLineBox(const FTransform& transform, const FColor& color, bool bOverlay = false);
	void DrawBox(const FTransform& transform, CMaterial* mat, bool bOverlay = false);

	void DrawLineCircle(const FTransform& transform, const FColor& color, bool bOverlay = false);
	void DrawCircle(const FTransform& transform, CMaterial* mat, bool bOverlay = false);

	void DrawText(const FVector2& screenPos, const FString& text, float time = 0.f, const FColor& color = FColor());
	
};
