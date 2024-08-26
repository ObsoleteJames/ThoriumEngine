
#include "SkyboxComponent.h"

void CSkyboxComponent::Init()
{
	BaseClass::Init();
	SetModel("models/Skybox.thasset");
	mat = CAssetManager::GetAsset<CMaterial>("materials/sky_default.thasset");
	SetMaterial(mat);
}

void CSkyboxComponent::OnMaterialChanged()
{
	SetMaterial(mat);
}
