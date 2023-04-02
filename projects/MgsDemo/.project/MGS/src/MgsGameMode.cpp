
#include "MgsGameMode.h"

#include "MgsPawn.h"

void CMgsGameMode::Init()
{
	BaseClass::Init();
	defaultPawnClass = CMgsPawn::StaticClass();
}
