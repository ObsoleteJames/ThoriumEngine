#pragma once

#include "Object/Object.h"
#include "Game/Entity.h"
#include "PawnController.generated.h"

class CPawn;

CLASS(Hidden)
class ENGINE_API CPawnController : public CEntity
{
	GENERATED_BODY()

public:
	CPawnController() = default;

public:
	void Possess(CPawn* pawn);
	inline CPawn* GetPawn() const { return pawn; }

private:
	TObjectPtr<CPawn> pawn;

};
