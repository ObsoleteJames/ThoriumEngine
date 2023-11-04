#pragma once

#include "ModelComponent.h"
#include "SkyboxComponent.generated.h"

CLASS()
class ENGINE_API CSkyboxComponent : public CModelComponent
{
	GENERATED_BODY()

public:
	CSkyboxComponent() = default;

	void Init() override;

private:
	FUNCTION()
	void OnMaterialChanged();

public:
	PROPERTY(Editable, OnEditFunc = OnMaterialChanged)
	TObjectPtr<CMaterial> mat;

};
