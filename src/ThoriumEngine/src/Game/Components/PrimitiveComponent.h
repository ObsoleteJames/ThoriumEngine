#pragma once

#include "SceneComponent.h"
#include "Game/Entity.h"
#include "PrimitiveComponent.generated.h"

class IPhysicsBody;

CLASS()
class ENGINE_API CPrimitiveComponent : public CSceneComponent
{
	GENERATED_BODY()

public:
	CPrimitiveComponent() = default;

	void Update(double dt) override;

	void SetVelocity(const FVector& v);
	FVector GetVelocity();

	inline bool IsKinematic() const { return GetEntity() ? (GetEntity()->GetType() == ENTITY_DYNAMIC && bStaticBody) : bStaticBody; }
	inline bool IsStatic() const { return GetEntity() ? GetEntity()->GetType() == ENTITY_STATIC : bStaticBody; }

	void EnableCollision(bool bEnabled);

	inline bool CollisionEnabled() const { return bEnableCollision; }

protected:
	virtual void UpdateWorldTransform();

	FUNCTION()
	inline void OnCollisionChanged() { EnableCollision(bEnableCollision); }

public:
	PROPERTY(Editable, Category = Physics, OnEditFunc = OnCollisionChanged)
	bool bEnableCollision = true;

	/*
	 * if true, this component will be either kinematic or completely static, depending on the owning entity's type.
	 * if false, this component will simulate physics, and its transform will be controlled by the physics body.
	*/
	PROPERTY(Editable, Category = Physics)
	bool bStaticBody = true;

	PROPERTY(Editable, Category = Physics)
	bool bWeldToParent = true;

	/*
	 * Wether this body will act as a trigger
	*/
	PROPERTY(Editable, Category = Physics, EditCondition = "bStaticBody")
	bool bIsTrigger = false;

	PROPERTY(Editable, Category = Physics, EditCondition = "bStaticBody")
	bool bEnableGravity = true;

protected:
	TObjectPtr<IPhysicsBody> physicsBody;
	bool bHasParentBody = false;

};
