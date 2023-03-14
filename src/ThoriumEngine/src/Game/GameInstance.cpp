
#include "GameInstance.h"

void CGameInstance::Init()
{
	AddLocalPlayer(0);
}

bool CGameInstance::AddLocalPlayer(uint controllerId)
{
	if (localPlayers.Size() == 4)
		return false;

	localPlayers.Add();
	FLocalPlayer& newPlayer = *localPlayers.last();

	newPlayer.controllerId = controllerId;
	newPlayer.playerId = localPlayers.Size() - 1;

	CPlayer* player = CreateObject<CPlayer>();
	players.Add(player);

	newPlayer.player = player;

	newPlayer.SetupViewport(localPlayers.Size(), false);
	return true;
}
