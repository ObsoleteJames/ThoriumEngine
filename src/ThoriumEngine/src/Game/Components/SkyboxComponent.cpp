
#include "SkyboxComponent.h"

void CSkyboxComponent::Init()
{
	BaseClass::Init();
	SetModel(L"models\\Skybox.thmdl");
	mat = CResourceManager::GetResource<CMaterial>(L"materials\\sky_default.thmat");
	SetMaterial(mat);
}

void CSkyboxComponent::OnMaterialChanged()
{
	SetMaterial(mat);
}
