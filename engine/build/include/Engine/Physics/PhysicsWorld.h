#pragma once

#include "Object/Object.h"
#include "ColliderShapes.h"
#include "PhysicsBody.h"
#include "HitInfo.h"
#include "PhysicsWorld.generated.h"

class IPhysicsBody;
class CEntity;
class CEntityComponent;

ENUM()
enum class EPhysicsLayer
{
	STATIC,
	DYNAMIC,
	COMPLEX, // Used for ray tracing
	TRIGGER,
	END
};

enum EPhysicsLayerFlag
{
	PhysicsLayer_Static = 1,
	PhysicsLayer_Dynamic = 1 << 1,
	PhysicsLayer_Complex = 1 << 2,
	PhysicsLayer_Trigger = 1 << 3,

	PhysicsLayer_All = PhysicsLayer_Static | PhysicsLayer_Dynamic | PhysicsLayer_Complex | PhysicsLayer_Trigger,
	PhysicsLayer_Common = PhysicsLayer_Static | PhysicsLayer_Dynamic
};

struct FPhysicsBodySettings
{
	CEntity* entity = nullptr;
	CEntityComponent* component = nullptr;

	EShapeType shapeType = SHAPE_INVALID;
	void* shapeData = nullptr;

	EPhysicsLayer physicsLayer = EPhysicsLayer::STATIC;
	EPhysicsBodyMotion motionType = PHBM_STATIC;

	FTransform transform;
};

CLASS(Abstract)
class ENGINE_API IPhysicsWorld : public CObject
{
	GENERATED_BODY()

public:
	virtual int Init() = 0;

	// Called before the world starts playing.
	virtual void Start() = 0;

	virtual void Update(double timestep) = 0;

	// Called after update, this function takes care of calling OnCollisionEnter/OnCollisionExit/OnCollisionStay on any bodies that have collided
	virtual void ResolveCollisions() = 0;

	virtual IPhysicsBody* CreateBody(const FPhysicsBodySettings&) = 0;

	virtual bool CastRay(const FRay& ray, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) = 0;
	virtual bool CastBox(const FVector& center, const FVector& size, const FQuaternion& rotation, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) = 0;
	virtual bool CastSphere(const FVector& center, float radius, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) = 0;
	virtual bool CastCapsule(const FVector& center, float radius, float height, const FQuaternion& rotation, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) = 0;
	
	virtual bool OverlapBox(const FVector& center, const FVector& size, const FQuaternion& rotation, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common) = 0;
	virtual bool OverlapSphere(const FVector& center, float radius, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common) = 0;
	virtual bool OverlapCapsule(const FVector& center, float radius, float height, const FQuaternion& rotation, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common) = 0;

	virtual void SetGravity(const FVector& v) = 0;
	virtual FVector GetGravity() = 0;
};
