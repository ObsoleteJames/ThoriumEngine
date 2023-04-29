#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "Game/Player.h"
#include "GameInstance.generated.h"

class CCanvas;

CLASS()
class ENGINE_API CGameInstance : public CObject
{
	GENERATED_BODY()

public:
	CGameInstance() = default;

	//virtual void PreInit() {}
	virtual void Init();

	virtual void Start();
	virtual void Stop();
	virtual void SpawnLocalPlayers();

public:
	inline const TArray<TObjectPtr<CPlayer>>& GetPlayers() const { return players; }
	inline const TArray<FLocalPlayer>& GetLocalPlayers() const { return localPlayers; }
	inline const TArray<TObjectPtr<CCanvas>>& GetGlobalCanvass() const { return globalCanvass; }

	FLocalPlayer* GetLocalPlayer(int index = 0);
	FLocalPlayer* GetLocalPlayer(CPlayer* p);

	bool AddLocalPlayer(uint controllerId = -1);

	void AddGlobalCanvas(CCanvas* canvas);
	void RemoveGlobalCanvas(CCanvas* canvas);

private:
	TArray<FLocalPlayer> localPlayers;
	TArray<TObjectPtr<CPlayer>> players;

	TArray<TObjectPtr<CCanvas>> globalCanvass;

};
