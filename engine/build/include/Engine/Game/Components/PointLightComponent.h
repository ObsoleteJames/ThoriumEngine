#pragma once

#include "EngineCore.h"
#include "Game/Components/SceneComponent.h"
#include "Math/Vectors.h"
#include "PointLightComponent.generated.h"

class CPrimitiveProxy;
class CLightProxy;

CLASS(Name = "Point Light")
class ENGINE_API CPointLightComponent : public CSceneComponent
{
	GENERATED_BODY()

public:
	void Init();
	void OnDelete();

public:
	PROPERTY(Editable)
	float intensity = 1.f;

	PROPERTY(Editable)
	float range = 10.f;

	PROPERTY(Editable)
	FVector color = FVector(1.f);

	PROPERTY(Editable)
	bool bCastShadows = true;

private:
	CLightProxy* lightProxy;

};
