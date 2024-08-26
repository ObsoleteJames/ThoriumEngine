
#include "SunLightEntity.h"
#include "Assets/TextureAsset.h"
#include "Game/Components/SunLightComponent.h"
#include "Game/Components/BillboardComponent.h"

void CSunLightEntity::Init()
{
	BaseClass::Init();

	light = AddComponent<CSunLightComponent>("Sun Light");
	light->AttachTo(RootComponent());

	TObjectPtr<CBillboardComponent> billboard = AddComponent<CBillboardComponent>("Billboard");
	billboard->AttachTo(light);
	billboard->SetSprite(CAssetManager::GetAsset<CTexture>("editor/icons/SunLight.thasset"));
	billboard->SetScale(FVector(0.36f));
	billboard->bEditorOnly = true;
}
