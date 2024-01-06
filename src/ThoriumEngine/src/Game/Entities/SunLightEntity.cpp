
#include "SunLightEntity.h"
#include "Resources/Texture.h"
#include "Game/Components/SunLightComponent.h"
#include "Game/Components/BillboardComponent.h"

void CSunLightEntity::Init()
{
	BaseClass::Init();

	light = AddComponent<CSunLightComponent>("Sun Light");
	light->AttachTo(RootComponent());

	TObjectPtr<CBillboardComponent> billboard = AddComponent<CBillboardComponent>("Billboard");
	billboard->AttachTo(light);
	billboard->sprite = CResourceManager::GetResource<CTexture>("editor/icons/SunLight.thtex");
	billboard->SetScale(FVector(0.36f));
	billboard->bEditorOnly = true;
}
