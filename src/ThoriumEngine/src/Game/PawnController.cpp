
#include "PawnController.h"

#include "Pawn.h"

void CPawnController::Possess(CPawn* target)
{
	if (!target || target->HasController())
		return;

	if (pawn)
		pawn->OnUnposses();

	pawn = target;
	pawn->SetOwner(this);
	pawn->OnPossessed(this);
}
