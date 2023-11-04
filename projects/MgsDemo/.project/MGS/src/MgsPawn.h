#pragma once

#include "EngineCore.h"
#include "Game/Pawn.h"
#include "MgsPawn.generated.h"

class CCameraComponent;

CLASS()
class CMgsPawn : public CPawn
{
	GENERATED_BODY()

public:
	void Init() override;

	void Update(double dt) override;

	void SetupInput(CInputManager* inputManager) override;

	void Tes();
	void Tab();

protected:
	void LookX(float v);
	void LookY(float v);

	void MoveForward(float v);
	void MoveStrafe(float v);

private:
	FVector look;

	CCameraComponent* cam;

};
