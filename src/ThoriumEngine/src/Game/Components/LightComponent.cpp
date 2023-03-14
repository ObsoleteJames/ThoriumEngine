
#include "LightComponent.h"
#include "Game/Entity.h"

void ILightComponent::Init()
{
	//FWorldRegisterer::RegisterLight(ent->GetWorld(), this);
}

void ILightComponent::OnDelete()
{
	//FWorldRegisterer::UnregisterLight(ent->GetWorld(), this);
}
