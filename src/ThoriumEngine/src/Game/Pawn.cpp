
#include "Pawn.h"
#include "Engine.h"
#include "Game/GameInstance.h"
#include "Game/Components/CameraComponent.h"

void CPawn::OnPossessed(const TObjectPtr<CPawnController>& controller)
{
	if (auto* player = gEngine->GameInstance()->GetLocalPlayer(0); player)
	{
		if (player->GetPlayer()->GetPlayerController() == controller)
		{
			auto cam = GetComponent<CCameraComponent>();
			if (cam)
				cam->MakePrimary();
		}
	}
}

void CPawn::OnUnposses()
{

}
