#pragma once

#include "Object/Object.h"
#include "PhysicsWorld.generated.h"

class IPhysicsBody;

ENUM()
enum class EPhysicsLayer
{
	STATIC,
	DYNAMIC,
	COMPLEX, // Used for ray tracing
	TRIGGER,

	CUSTOM_0,
	CUSTOM_1,
	CUSTOM_2,
	CUSTOM_3,
	CUSTOM_4,
	CUSTOM_5,

	END
};

CLASS(Abstract)
class ENGINE_API IPhysicsWorld : public CObject
{
	GENERATED_BODY()

public:
	virtual int Init() = 0;

	// Called before the world starts playing.
	virtual void Start() = 0;

	virtual void StartSim(double timestep) = 0;
	virtual void StopSim() = 0;

	virtual IPhysicsBody* CreateDynamicBody() = 0;
	virtual IPhysicsBody* CreateStaticBody() = 0;

};
