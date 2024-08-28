#pragma once

#include "PawnController.h"
#include "PlayerController.generated.h"

class CPlayer;
class CCanvas;
class CRenderScene;

CLASS(Hidden)
class ENGINE_API CPlayerController : public CPawnController 
{
	GENERATED_BODY()

	friend class CGameMode;
	friend class CPlayer;

public:
	CPlayerController();

	inline CPlayer* GetPlayer() const { return player; }
	inline const TArray<TObjectPtr<CCanvas>>& GetCanvass() const { return canvass; }

	void AddCanvas(CCanvas* canvas);
	void RemoveCanvas(CCanvas* canvas);

protected:
	TObjectPtr<CPlayer> player;
	TArray<TObjectPtr<CCanvas>> canvass;
};
