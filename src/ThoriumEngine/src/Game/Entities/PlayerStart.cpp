
#include "PlayerStart.h"
#include "Game/Components/ModelComponent.h"

void CPlayerStart::Init()
{
	BaseClass::Init();

	mdl = AddComponent<CModelComponent>("model");
	mdl->AttachTo(RootComponent());
	mdl->SetModel("models/Dummy/Dummy.thmdl");
	mdl->SetMaterial("models/Dummy/DummyMatGreen.thmat");
	mdl->bEditorOnly = true;
}
