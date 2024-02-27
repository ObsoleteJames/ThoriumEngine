#pragma once

#include "Object/Object.h"
#include "Math/Math.h"
#include "Math/Vectors.h"
#include "HitInfo.generated.h"

class CPrimitiveComponent;
class CEntity;
class IPhysicsBody;

STRUCT()
struct FHitInfo
{
	GENERATED_BODY()

public:
	PROPERTY()
	TObjectPtr<CPrimitiveComponent> component;

	PROPERTY()
	TObjectPtr<IPhysicsBody> body;

	PROPERTY()
	TObjectPtr<CEntity> entity;

	PROPERTY()
	FVector position;

	PROPERTY()
	FVector normal;

	PROPERTY()
	float distance;

};
