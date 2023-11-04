#pragma once

#include "Object/Object.h"
#include "PhysicsApi.generated.h"

class IPhysicsWorld;

CLASS(Abstract)
class IPhysicsApi : public CObject
{
	GENERATED_BODY()

public:
	virtual ~IPhysicsApi() = default;

	virtual int Init() = 0;
	virtual void Shutdown() = 0;

	virtual IPhysicsWorld* CreateWorld() = 0;
	virtual void DestroyWorld(IPhysicsWorld* world) = 0;

};
