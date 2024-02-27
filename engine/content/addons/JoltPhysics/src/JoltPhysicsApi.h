#pragma once

#include "Physics/PhysicsApi.h"
#include "JoltPhysics.h"
#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>

#include "JoltPhysicsApi.generated.h"

#define FVECTOR_TO_JPH(Vec) JPH::Vec3(Vec.x, Vec.y, Vec.z)
#define JPH_TO_FVECTOR(Jph) FVector(Jph.GetX(), Jph.GetY(), Jph.GetZ())

CLASS()
class JOLTPHYSICS_API CJoltPhysicsApi : public IPhysicsApi
{
	GENERATED_BODY()

public:
	int Init() final;
	void Shutdown() final;

	IPhysicsWorld* CreateWorld() final;
	void DestroyWorld(IPhysicsWorld* world) final;

public:
	TArray<IPhysicsWorld*> worlds;

	JPH::JobSystemThreadPool* jobSystem;
	JPH::TempAllocatorImpl* tempAllocator;
};

inline CJoltPhysicsApi* GetJoltPhysicsApi() { return (CJoltPhysicsApi*)gPhysicsApi; }

//CLASS()
//class JOLTPHYSICS_API CJoltPhysicsApi : public IPhysicsWorld
//{
//	void Init() final;
//
//	void Start() final;
//
//	void StartSim(double timestep) final;
//	void StopSim() final;
//
//	IPhysicsBody* CreateBody(const FPhysicsBodySettings&) final;
//
//	void SetGravity(const FVector& g) final;
//	FVector GetGravity() final;
//};
