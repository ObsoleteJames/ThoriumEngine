#pragma once

#include "Object/Object.h"
#include "Player.generated.h"

class CPawn;
class CPlayerController;

CLASS()
class ENGINE_API CPlayer : public CObject 
{
	GENERATED_BODY()

	friend class CGameInstance;
	friend class CGameMode;

public:
	CPlayer() = default;

	//FUNCTION(ServerRpc)
	void SetPlayerController(CPlayerController* newController);

	inline CPlayerController* GetPlayerController() const { return controller; }
	inline CPawn* GetPawn() const { return controller->GetPawn(); }

private:
	//FUNCTION(ClientRpc) // TODO implement
	void clPlayerControllerUpdate(CPlayerController* newController) {}

private:
	TObjectPtr<CPlayerController> controller;
	SizeType netId;

};

class ENGINE_API FLocalPlayer
{
	friend class CGameInstance;

public:
	FLocalPlayer() = default;

	void SetupViewport(uint8 numPlayers, bool bSplitVertical);

private:
	CPlayer* player;
	uint8 playerId;
	uint8 controllerId;

	float viewRect[4]; // 0 - 1

};
