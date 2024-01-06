
#include "SkyboxComponent.h"

void CSkyboxComponent::Init()
{
	BaseClass::Init();
	SetModel("models/Skybox.thmdl");
	mat = CResourceManager::GetResource<CMaterial>("materials/sky_default.thmat");
	SetMaterial(mat);
}

void CSkyboxComponent::OnMaterialChanged()
{
	SetMaterial(mat);
}
