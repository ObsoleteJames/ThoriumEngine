#pragma once

#include "EngineCore.h"
#include "physx_4.1.h"
#include "Physics/PhysicsApi.h"
#include "PhysxApi.generated.h"

CLASS()
class PHYSX_4_1_API CPhysXApi : public IPhysicsApi
{
	GENERATED_BODY()

public:
	int Init() override;
	void Shutdown() override;

	IPhysicsWorld* CreateWorld() override;
	void DestroyWorld(IPhysicsWorld* world) override;
}; 
