#pragma once

#include "Game/Entity.h"
#include "SunLightEntity.generated.h"

class CSunLightComponent;

CLASS(Name = "Sun Light")
class ENGINE_API CSunLightEntity : public CEntity
{
	GENERATED_BODY()

public:
	void Init();

public:
	CSunLightComponent* light;
};
