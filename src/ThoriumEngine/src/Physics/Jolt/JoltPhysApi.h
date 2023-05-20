#pragma once

#include "Physics/PhysicsApi.h"
#include "JoltPhysApi.generated.h"

CLASS()
class CJoltPhysApi : public IPhysicsApi
{
	GENERATED_BODY()

public:
	int Init() override final;
	void Shutdown() override final;

	IPhysicsWorld* CreateWorld() override final;
	void DestroyWorld(IPhysicsWorld* world) override final;

};
