#pragma once

#include "Game/Entity.h"
#include "Game/Components/ModelComponent.h"
#include "ModelEntity.generated.h"

class CModelComponent;
class CModelAsset;

CLASS()
class ENGINE_API CModelEntity : public CEntity
{
	GENERATED_BODY()

public:
	virtual void Init();

	inline void SetModel(CModelAsset* mdl) { modelComp->SetModel(mdl); }

public:
	CModelComponent* modelComp;

};
