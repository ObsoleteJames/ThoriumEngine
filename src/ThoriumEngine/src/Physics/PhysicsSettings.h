#pragma once

#include "PhysicsApi.h"
#include "Math/Vectors.h"

struct FPhysicsSettings
{
	TClassPtr<IPhysicsApi> api;
	FVector gravity;

};
