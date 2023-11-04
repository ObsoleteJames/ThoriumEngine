#pragma once

#include "EngineCore.h"
#include "Game/Components/SceneComponent.h"
#include "Math/Vectors.h"
#include "SunLightComponent.generated.h"

class CLightProxy;

CLASS()
class ENGINE_API CSunLightComponent : public CSceneComponent
{
	GENERATED_BODY()

public:
	void Init();
	void OnDelete();

public:
	PROPERTY(Editable)
	float intensity = 5.f;

	PROPERTY(Editable)
	FVector color = FVector(1.f);

	PROPERTY(Editable)
	bool bCastShadows = true;

	PROPERTY(Editable, Name = "Shadow Bias", UIStepSize = 0.000001, UIFormat = "%.5f")
	float shadowBias = 0.0002f;

private:
	CLightProxy* lightProxy;

};
