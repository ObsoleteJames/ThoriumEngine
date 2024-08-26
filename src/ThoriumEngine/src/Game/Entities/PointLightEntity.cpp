
#include "PointLightEntity.h"
#include "Assets/TextureAsset.h"
#include "Game/Components/PointLightComponent.h"
#include "Game/Components/BillboardComponent.h"

void CPointLightEntity::Init()
{
	BaseClass::Init();

	light = AddComponent<CPointLightComponent>("Point Light");
	light->AttachTo(RootComponent());

	TObjectPtr<CBillboardComponent> billboard = AddComponent<CBillboardComponent>("Billboard");
	billboard->AttachTo(light);
	billboard->sprite = CAssetManager::GetAsset<CTexture>("editor/icons/PointLight.thtex");
	billboard->SetScale(FVector(0.36f));
	billboard->bEditorOnly = true;
}
