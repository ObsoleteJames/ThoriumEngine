#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "Game/Player.h"
#include "GameInstance.generated.h"

CLASS()
class ENGINE_API CGameInstance : public CObject
{
	GENERATED_BODY()

public:
	CGameInstance() = default;

	virtual void PreInit() {}
	virtual void Init();

public:
	inline const TArray<CPlayer*>& GetPlayers() const { return players; }
	inline const TArray<FLocalPlayer>& GetLocalPlayers() const { return localPlayers; }

	inline FLocalPlayer& GetLocalPlayer(int index = 0) { return localPlayers[index]; }

	bool AddLocalPlayer(uint controllerId);

private:
	TArray<FLocalPlayer> localPlayers;
	TArray<CPlayer*> players;

};
