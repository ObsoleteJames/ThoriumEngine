#pragma once

#include "Entity.h"
#include "Pawn.generated.h"

class CPawnController;

CLASS()
class ENGINE_API CPawn : public CEntity
{
	GENERATED_BODY()

public:
	virtual void OnPossessed(const TObjectPtr<CPlayerController>& player);
	virtual void OnUnposses();

	inline CPawnController* GetController() const { return controller; }

private:
	TObjectPtr<CPawnController> controller;

};
