
#include "PrimitiveComponent.h"
#include "Physics/PhysicsBody.h"

void CPrimitiveComponent::SetVelocity(const FVector& v)
{
	if (physicsBody)
		physicsBody->SetVelocity(v);
}

FVector CPrimitiveComponent::GetVelocity()
{
	if (physicsBody)
		return physicsBody->GetVelocity();

	return FVector::zero;
}

void CPrimitiveComponent::EnableCollision(bool bEnabled)
{
	if (physicsBody)
		physicsBody->SetEnabled(bEnabled);
	bEnableCollision = bEnabled;

	for (TObjectPtr<CSceneComponent> child : GetChildren())
	{
		if (CPrimitiveComponent* prim = CastChecked<CPrimitiveComponent>(child); prim != nullptr)
		{
			prim->EnableCollision(bEnabled);
		}
	}
}

void CPrimitiveComponent::Update(double dt)
{
	if (physicsBody)
	{
		if (physicsBody->IsAwake() && !bHasParentBody)
		{
			CSceneComponent* r = GetEntity()->RootComponent();
			r->SetPosition(physicsBody->GetPosition());
			r->SetRotation(physicsBody->GetRotation());
		}
	}
}

void CPrimitiveComponent::UpdateWorldTransform()
{
	BaseClass::UpdateWorldTransform();

	if (physicsBody && !bHasParentBody)
	{
		physicsBody->SetPosition(GetWorldPosition());
		physicsBody->SetRotation(GetWorldRotation());
	}
}
