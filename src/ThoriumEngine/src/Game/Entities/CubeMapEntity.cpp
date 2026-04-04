
#include "CubeMapEntity.h"
#include "Assets/TextureAsset.h"
#include "Game/Components/CubeMapComponent.h"
#include "Game/Components/BillboardComponent.h"

void CCubeMapEntity::Init()
{
	BaseClass::Init();

	cubemap = AddComponent<CCubeMapComponent>("Cube Map Volume");
	cubemap->AttachTo(RootComponent());

	TObjectPtr<CBillboardComponent> billboard = AddComponent<CBillboardComponent>("Billboard");
	billboard->AttachTo(cubemap);
	billboard->SetSprite(CAssetManager::GetAsset<CTexture>("editor/icons/env_cubemap.thasset"));
	billboard->SetScale(FVector(0.36f));
	billboard->bEditorOnly = true;
	billboard->renderLayer = R_LAYER_EDITOR;
}
