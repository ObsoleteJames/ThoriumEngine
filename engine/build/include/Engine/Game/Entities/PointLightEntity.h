#pragma once

#include "Game/Entity.h"
#include "PointLightEntity.generated.h"

class CPointLightComponent;

CLASS(Name = "Point Light")
class ENGINE_API CPointLightEntity : public CEntity
{
	GENERATED_BODY()

public:
	virtual void Init();

public:
	CPointLightComponent* light;

};
