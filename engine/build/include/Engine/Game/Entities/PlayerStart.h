#pragma once

#include "Game/Entity.h"
#include "PlayerStart.generated.h"

class CModelComponent;

CLASS()
class ENGINE_API CPlayerStart : public CEntity
{
	GENERATED_BODY();
public:
	void Init() override;
	
public:
	PROPERTY(Editable)
	FString tag;

private:
	TObjectPtr<CModelComponent> mdl;

};
