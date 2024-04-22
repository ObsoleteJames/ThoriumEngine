#pragma once

#include "Game/Entity.h"
#include "PhysicsBody.generated.h"

class IPhysicsWorld;

ENUM()
enum EPhysicsBodyMotion
{
	PHBM_STATIC,
	PHBM_DYNAMIC,
	PHBM_KINEMATIC
};

CLASS(Abstract)
class ENGINE_API IPhysicsBody : public CObject
{
	GENERATED_BODY()

public:
	virtual void SetPosition(const FVector& p) = 0;
	virtual FVector GetPosition() = 0;

	virtual void SetScale(const FVector& s) = 0;
	virtual FVector GetScale() = 0;

	virtual void SetRotation(const FQuaternion& r) = 0;
	virtual FQuaternion GetRotation() = 0;

	virtual void SetVelocity(const FVector& v) = 0;
	virtual FVector GetVelocity() = 0;
	virtual FVector GetVelocity(const FVector& point) = 0;

	virtual void SetAngularVelocity(const FVector& v) = 0;
	virtual FVector GetAngularVelocity() = 0;

	virtual void MoveTo(const FVector& position, const FQuaternion& rotation) = 0;

	virtual bool IsAwake() = 0;
	virtual void Wake() = 0;

	virtual void SetMotionType(EPhysicsBodyMotion type) = 0;

	virtual void SetEnabled(bool bEnabled) = 0;
	virtual bool IsEnabled() const = 0;

	virtual void AddForce(const FVector& v, const FVector& point = FVector::zero) = 0;
	virtual void AddTorque(const FVector& v) = 0;

	virtual void AddImpulse(const FVector& v, const FVector& point = FVector::zero) = 0;

	inline CEntity* GetEntity() const { return CastChecked<CEntity>(GetOwner()); }
	inline CEntityComponent* GetEntityComponent() const { return component; }

protected:
	TObjectPtr<IPhysicsWorld> world;

	// the component that owns this physics body
	TObjectPtr<CEntityComponent> component;
};
