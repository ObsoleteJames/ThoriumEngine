
#include "JoltPhysicsApi.h"
#include "JoltPhysicsWorld.h"
#include "Module.h"

#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include <thread>

int CJoltPhysicsApi::Init()
{
	JPH::RegisterDefaultAllocator();

	JPH::Factory::sInstance = new JPH::Factory();

	JPH::RegisterTypes();

	tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
	jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

	return 1;
}

void CJoltPhysicsApi::Shutdown()
{
	for (auto* w : worlds)
		w->Delete();

	JPH::UnregisterTypes();

	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

IPhysicsWorld* CJoltPhysicsApi::CreateWorld()
{
	CJoltPhysicsWorld* world = CreateObject<CJoltPhysicsWorld>();
	if (world->Init() == 0)
	{
		world->Delete();
		return nullptr;
	}
	worlds.Add(world);
	return world;
}

void CJoltPhysicsApi::DestroyWorld(IPhysicsWorld* world)
{
	world->Delete();
	worlds.Erase(worlds.Find(world));
}
