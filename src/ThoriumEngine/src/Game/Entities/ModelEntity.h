#pragma once

#include "Game/Entity.h"
#include "ModelEntity.generated.h"

class CModelComponent;

CLASS()
class ENGINE_API CModelEntity : public CEntity
{
	GENERATED_BODY()

public:
	virtual void Init();

public:
	CModelComponent* modelComp;

};
