
#include "EntityComponent.h"
#include "Game/Entity.h"

CWorld* CEntityComponent::GetWorld() const
{
	return ent->GetWorld();
}

bool CEntityComponent::IsVisible() const
{
	return ent ? (ent->bIsVisible && bIsVisible) : bIsVisible;
}

void CEntityComponent::OnDelete()
{
	if (ent) 
		ent->RemoveComponent(this);
}
