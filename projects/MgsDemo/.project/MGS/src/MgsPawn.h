#pragma once

#include "EngineCore.h"
#include "Game/Pawn.h"
#include "MgsPawn.generated.h"

CLASS()
class CMgsPawn : public CPawn
{
	GENERATED_BODY()

public:
	void Init() override;

	void Update(double dt) override;

};
