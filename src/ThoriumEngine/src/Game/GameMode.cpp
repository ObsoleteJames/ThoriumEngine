
#include "GameMode.h"
#include "Player.h"
#include "PlayerController.h"
#include "Pawn.h"
#include "Engine.h"
#include "Game/Input/InputManager.h"
#include "Game/Components/CameraComponent.h"
#include "Game/GameInstance.h"
#include "Game/World.h"

#include "Game/Entities/PlayerStart.h"

void CGameMode::Init()
{
	playerControllerClass = CPlayerController::StaticClass();
	defaultPawnClass = CPawn::StaticClass();
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
	controller->SetOwner(player);
	playerControllers.Add(controller);

	SpawnPlayer(player);
}

void CGameMode::OnPlayerDisconnect(CPlayer* player)
{
	// Do Cleanup
	if (!player->GetPlayerController())
		return;

	playerControllers.Erase(playerControllers.Find(player->GetPlayerController()));

	player->GetPawn()->Delete();
	auto oldPC = player->GetPlayerController();
	player->SetPlayerController(nullptr);
	oldPC->Delete();
}

void CGameMode::SpawnPlayer(CPlayer* player)
{
	if (!defaultPawnClass.Get())
		return;

	// Spawn a pawn for the player.
	TObjectPtr<CPawn> pawn = (CPawn*)GetWorld()->CreateEntity(defaultPawnClass.Get(), FString());
	if (!pawn)
		return;

	player->GetPlayerController()->Possess(pawn);
	//gInputManager->RegisterPlayer(player);

	FVector pos;
	FQuaternion rot;
	FindPlayerSpawnPoint(player, pawn, pos, rot);

	pawn->SetWorldPosition(pos);
	pawn->SetWorldRotation(rot);
}

void CGameMode::FindPlayerSpawnPoint(CPlayer* player, CPawn* pawn, FVector& outPosition, FQuaternion& outRotation)
{
	auto spawnPoints = GetWorld()->FindEntitiesOfType<CPlayerStart>();
	if (spawnPoints.Size() == 0)
		return;

	int rng = FMath::Random(spawnPoints.Size());

	outPosition = spawnPoints[rng]->GetWorldPosition();
	outRotation = spawnPoints[rng]->GetWorldRotation();
}

TObjectPtr<CPlayerController> CGameMode::GetPlayerController(SizeType id)
{
	if (id >= playerControllers.Size())
		return nullptr;
	return playerControllers[id];
}

void CGameMode::OnDelete()
{
	for (auto it = playerControllers.rbegin(); it != playerControllers.rend(); it++)
		OnPlayerDisconnect((*it)->GetPlayer());
}
