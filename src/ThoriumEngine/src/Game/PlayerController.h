#pragma once

#include "PawnController.h"
#include "PlayerController.generated.h"

CLASS(Hidden)
class ENGINE_API CPlayerController : public CPawnController 
{
	GENERATED_BODY()

	friend class CGameMode;

public:
	inline CPlayer* GetPlayer() const { return player; }

protected:
	TObjectPtr<CPlayer> player;

};
