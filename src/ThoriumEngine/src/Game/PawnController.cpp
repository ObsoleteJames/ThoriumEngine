
#include "PawnController.h"

#include "Pawn.h"

void CPawnController::Possess(const TObjectPtr<CPawn>& target)
{
	if (target->GetController())
		return;

	if (pawn)
		pawn->OnUnposses();

	pawn = target;
	pawn->SetOwner(this);
	pawn->OnPossessed(this);
}
