
#include "EntityComponent.h"
#include "Game/Entity.h"

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
}
