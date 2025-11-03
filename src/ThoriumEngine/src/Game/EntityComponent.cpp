
#include "EntityComponent.h"
#include "Game/Entity.h"

void CEntityComponent::Init()
{
	if (bRequireUpdate)
		GetWorld()->OnUpdate.Bind(this, &CEntityComponent::DoUpdate);
}

CWorld* CEntityComponent::GetWorld() const
{
	return ent->GetWorld();
}

bool CEntityComponent::IsVisible() const
{
	return ent ? (ent->bIsEnabled && ent->bIsVisible && bIsVisible) : bIsVisible;
}

void CEntityComponent::SetComponentId(SizeType id)
{
	ent->components.erase(compId);
	ent->components[id] = this;

	compId = id;
}

void CEntityComponent::OnDelete()
{
	if (ent) 
		ent->RemoveComponent(this);

	if (bRequireUpdate)
		GetWorld()->OnUpdate.RemoveAll(this);
}

void CEntityComponent::DoUpdate(double dt)
{
	if (ent)
	{
		if (ent->bIsEnabled)
			Update(dt);
	}
	else
		Update(dt);
}
