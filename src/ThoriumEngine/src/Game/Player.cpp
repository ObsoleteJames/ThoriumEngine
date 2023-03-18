
#include "Player.h"

void FLocalPlayer::SetupViewport(uint8 numPlayers, bool bSplitVertical)
{

}

void CPlayer::SetPlayerController(CPlayerController* newController)
{
	if (controller)
		controller->Delete();

	controller = newController;

	if (controller)
		controller->player = this;
}
