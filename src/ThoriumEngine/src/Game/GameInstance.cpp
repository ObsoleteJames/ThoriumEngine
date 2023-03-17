
#include "GameInstance.h"
#include "World.h"
#include "GameMode.h"

void CGameInstance::Init()
{
	AddLocalPlayer(0);
}

void CGameInstance::OnStart()
{

}

void CGameInstance::OnStop()
{

}

void CGameInstance::SpawnLocalPlayers()
{
	CGameMode* gamemode = gWorld->GetGameMode();

	for (auto& lp : localPlayers)
	{
		gamemode->OnPlayerJoined(lp.player);
	}
}

bool CGameInstance::AddLocalPlayer(uint controllerId)
{
	if (localPlayers.Size() == 4)
		return false;

	localPlayers.Add();
	FLocalPlayer& newPlayer = *localPlayers.last();

	newPlayer.controllerId = controllerId == -1 ? localPlayers.Size() - 1 : controllerId;
	newPlayer.playerId = localPlayers.Size() - 1;

	CPlayer* player = CreateObject<CPlayer>();
	players.Add(player);

	newPlayer.player = player;

	newPlayer.SetupViewport(localPlayers.Size(), false);
	return true;
}
