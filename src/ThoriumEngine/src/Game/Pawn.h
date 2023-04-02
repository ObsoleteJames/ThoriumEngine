#pragma once

#include "Entity.h"
#include "Pawn.generated.h"

class CPawnController;

CLASS()
class ENGINE_API CPawn : public CEntity
{
	GENERATED_BODY()

public:
	virtual void OnPossessed(const TObjectPtr<CPawnController>& controller);
	virtual void OnUnposses();

	inline CPawnController* GetController() const { return controller; }
	inline bool HasController() const { return controller.IsValid(); }

private:
	TObjectPtr<CPawnController> controller;

};
