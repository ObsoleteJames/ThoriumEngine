
#include "PlayerStart.h"
#include "Game/Components/ModelComponent.h"

void CPlayerStart::Init()
{
	BaseClass::Init();

	mdl = AddComponent<CModelComponent>("model");
	mdl->AttachTo(RootComponent());
	mdl->SetModel("models/Dummy/Dummy.thasset");
	mdl->SetMaterial("models/Dummy/DummyMatGreen.thasset");
	mdl->bEditorOnly = true;
}
