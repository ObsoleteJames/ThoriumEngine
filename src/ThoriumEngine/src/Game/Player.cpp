
#include "Player.h"
#include "Engine.h"
#include "GameInstance.h"

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

bool CPlayer::IsLocalPlayer()
{
	return gEngine->GameInstance()->GetLocalPlayer(this) != nullptr;
}
