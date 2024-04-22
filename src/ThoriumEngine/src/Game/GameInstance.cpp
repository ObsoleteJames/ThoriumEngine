
#include "GameInstance.h"
#include "World.h"
#include "GameMode.h"
#include "Engine.h"

void CGameInstance::Init()
{
	if (gIsClient)
		AddLocalPlayer(0);
}

void CGameInstance::Start()
{

}

void CGameInstance::Stop()
{

}

CGameInstance* CGameInstance::Get()
{
	return gEngine->GameInstance();
}

void CGameInstance::SpawnLocalPlayers()
{
	CGameMode* gamemode = gWorld->GetGameMode();

	for (auto& lp : localPlayers)
	{
		gamemode->OnPlayerJoined(lp.player);
	}
}

FLocalPlayer* CGameInstance::GetLocalPlayer(int index /*= 0*/)
{
	if (index >= localPlayers.Size()) 
		return nullptr; 
	return &localPlayers[index];
}

FLocalPlayer* CGameInstance::GetLocalPlayer(CPlayer* p)
{
	for (auto& pl : localPlayers)
	{
		if (pl.player == p)
			return &pl;
	}
	return nullptr;
}

bool CGameInstance::AddLocalPlayer(uint controllerId)
{
	if (localPlayers.Size() == 4)
		return false;

	localPlayers.Add();
	FLocalPlayer& newPlayer = *localPlayers.last();

	newPlayer.controllerId = (uint8)(controllerId == -1 ? localPlayers.Size() - 1 : controllerId);
	newPlayer.playerId = (uint8)localPlayers.Size() - 1;

	CPlayer* player = CreateObject<CPlayer>();
	players.Add(player);

	newPlayer.player = player;

	newPlayer.SetupViewport((uint8)localPlayers.Size(), false);
	return true;
}

void CGameInstance::AddGlobalCanvas(CCanvas* canvas)
{
	globalCanvass.Add(canvas);
}

void CGameInstance::RemoveGlobalCanvas(CCanvas* canvas)
{
	if (auto it = globalCanvass.Find(canvas); it != globalCanvass.end())
		globalCanvass.Erase(it);
}
