#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "Math/Vectors.h"
#include "GameMode.generated.h"

class CPlayer;
class CPawn;
class CWorld;
class CPlayerController;

CLASS()
class ENGINE_API CGameMode : public CObject
{
	GENERATED_BODY()

	friend class CWorld;
	friend class CGameInstance;

public:
	CGameMode() = default;

	virtual void Init();

	virtual void OnStart();
	virtual void Update(double dt);

public:
	virtual void OnPlayerJoined(CPlayer* player);
	virtual void OnPlayerDisconnect(CPlayer* player);
	virtual void SpawnPlayer(CPlayer* player);

	virtual void FindPlayerSpawnPoint(CPlayer* player, CPawn* pawn, FVector& outPosition, FQuaternion& outRotation);

public:
	inline const TArray<TObjectPtr<CPlayerController>>& GetPlayerControllers() const { return playerControllers; }
	TObjectPtr<CPlayerController> GetPlayerController(SizeType index);

	inline CWorld* World() const { return world; }

protected:
	virtual void OnDelete();

protected:
	PROPERTY(Editable)
	TClassPtr<CPlayerController> playerControllerClass;

	PROPERTY(Editable)
	TClassPtr<CPawn> defaultPawnClass;

	TArray<TObjectPtr<CPlayerController>> playerControllers;

private:
	TObjectPtr<CWorld> world;

};
