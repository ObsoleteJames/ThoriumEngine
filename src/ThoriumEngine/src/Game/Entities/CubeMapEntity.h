#pragma once

#include "Game/Entity.h"
#include "CubeMapEntity.generated.h"

class CCubeMapComponent;

CLASS(Name = "Cube Map")
class ENGINE_API CCubeMapEntity : public CEntity
{
	GENERATED_BODY()

public:
	void Init();

public:
	PROPERTY(DontSerialize, ExposeProperties)
	TObjectPtr<CCubeMapComponent> cubemap;
};
