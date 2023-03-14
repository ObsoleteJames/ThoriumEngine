
#include "ModelEntity.h"
#include "Game/Components/ModelComponent.h"

void CModelEntity::Init()
{
	BaseClass::Init();

	modelComp = AddComponent<CModelComponent>("Model");
	modelComp->AttachTo(RootComponent());
}
