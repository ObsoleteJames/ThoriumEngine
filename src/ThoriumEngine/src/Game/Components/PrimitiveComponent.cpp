
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
