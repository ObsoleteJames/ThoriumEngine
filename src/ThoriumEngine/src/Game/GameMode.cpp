
#include "GameMode.h"
#include "Player.h"
#include "PlayerController.h"
#include "Pawn.h"
#include "Game/World.h"

void CGameMode::Init()
{

}

void CGameMode::OnStart()
{
}

void CGameMode::OnPlayerJoined(CPlayer* player)
{
	// Give the player a PlayerController.
	if (player->GetPlayerController())
		player->SetPlayerController(nullptr);

	TObjectPtr<CPlayerController> controller = (CPlayerController*)GetWorld()->CreateEntity(playerControllerClass.Get(), FString());
	if (!controller)
		return;

	player->SetPlayerController(controller);
	playerControllers.Add(controller);
}

void CGameMode::OnPlayerDisconnect(CPlayer* player)
{
	// Do Cleanup
	if (!player->GetPlayerController())
		return;

	playerControllers.Erase(playerControllers.Find(player->GetPlayerController()));
	player->GetPlayerController()->Delete();
}

void CGameMode::SpawnPlayer(CPlayer* player)
{
	// Spawn a pawn for the player.
	TObjectPtr<CPawn> pawn = (CPawn*)GetWorld()->CreateEntity(defaultPawnClass.Get(), FString());
	if (!pawn)
		return;

	player->GetPlayerController()->Possess(pawn);
}

TObjectPtr<CPlayerController> CGameMode::GetPlayerController(SizeType id)
{
	if (id >= playerControllers.Size())
		return nullptr;
	return playerControllers[id];
}

void CGameMode::OnDelete()
{
	for (auto& pc : playerControllers)
		pc->Delete();

	playerControllers.Clear();
}
