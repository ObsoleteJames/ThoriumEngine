
#include "PlayerStart.h"
#include "Game/Components/ModelComponent.h"

void CPlayerStart::Init()
{
	BaseClass::Init();

	mdl = AddComponent<CModelComponent>("model");
	mdl->AttachTo(RootComponent());
	mdl->SetModel(L"models\\PlayerStart.thmdl");
	mdl->bEditorOnly = true;
}
