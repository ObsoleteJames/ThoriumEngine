#pragma once

#include "JoltPhysicsApi.h"
#include "Physics/PhysicsWorld.h"

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
#include "JoltPhysicsWorld.generated.h"

static constexpr JPH::BroadPhaseLayer bpLayerStatic((int)EPhysicsLayer::STATIC);
static constexpr JPH::BroadPhaseLayer bpLayerDynamic((int)EPhysicsLayer::DYNAMIC);
static constexpr JPH::BroadPhaseLayer bpLayerComplex((int)EPhysicsLayer::COMPLEX);
static constexpr JPH::BroadPhaseLayer bpLayerTrigger((int)EPhysicsLayer::TRIGGER);

class CJoltPhysicsBody;

class BPLayerInterfaceImpl : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		auto* e = CModuleManager::FindEnum("EPhysicsLayer");
		if (e)
		{
			for (auto& v : e->GetValues())
			{
				BPNames[v.Value] = v.Key;
			}
		}
	}

	virtual uint GetNumBroadPhaseLayers() const override { return (uint)EPhysicsLayer::END; }

	virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
	{
		return JPH::BroadPhaseLayer(inLayer);
	}

	virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override
	{
		return BPNames[inLayer.GetValue()].c_str();
	}

	FString BPNames[(int)EPhysicsLayer::END];
};

class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer layer1, JPH::ObjectLayer layer2) const override
	{
		switch (layer1)
		{
		case (int)EPhysicsLayer::STATIC:
			return layer2 == (int)EPhysicsLayer::DYNAMIC;
		case (int)EPhysicsLayer::DYNAMIC:
			return layer2 != (int)EPhysicsLayer::COMPLEX;
		case (int)EPhysicsLayer::COMPLEX:
			return false;
		case (int)EPhysicsLayer::TRIGGER:
			return layer2 != (int)EPhysicsLayer::COMPLEX;
		}

		return false;
	}
};

class ObjVsBPLayerImpl : public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	virtual bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const override
	{
		switch (layer1)
		{
		case (int)EPhysicsLayer::STATIC:
			return layer2 == bpLayerDynamic;
		case (int)EPhysicsLayer::DYNAMIC:
			return layer2 != bpLayerComplex;
		case (int)EPhysicsLayer::COMPLEX:
			return false;
		case (int)EPhysicsLayer::TRIGGER:
			return layer2 != bpLayerComplex;
		}

		return false;
	}
};

class ThContactListener : public JPH::ContactListener
{
public:
	virtual JPH::ValidateResult OnContactValidate(const JPH::Body& body1, const JPH::Body& body2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& collResult) override
	{
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
	{
	}

	virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
	{
	}

	virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
	{
	}
};

class ThBodyActivationListener : public JPH::BodyActivationListener
{
public:
	virtual void OnBodyActivated(const JPH::BodyID& inBodyID, uint64 inBodyUserData) override
	{
	}

	virtual void OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64 inBodyUserData) override
	{
	}
};

CLASS()
class JOLTPHYSICS_API CJoltPhysicsWorld : public IPhysicsWorld
{
	GENERATED_BODY()

public:
	// -- IPhysicsWorld interface --
	int Init() final;

	void Start() final;

	void Update(double timestep) final;
	
	void ResolveCollisions() final;

	IPhysicsBody* CreateBody(const FPhysicsBodySettings&) final;

	bool CastRay(const FRay& ray, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) final;
	bool CastBox(const FVector& center, const FVector& size, const FQuaternion& rotation, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) final;
	bool CastSphere(const FVector& center, float radius, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) final;
	bool CastCapsule(const FVector& center, float radius, float height, const FQuaternion& rotation, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common, float maxDistance = FLT_MAX) final;
	
	bool OverlapBox(const FVector & center, const FVector & size, const FQuaternion & rotation, TArray<IPhysicsBody*>&outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common) final;
	bool OverlapSphere(const FVector& center, float radius, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common) final;
	bool OverlapCapsule(const FVector& center, float radius, float height, const FQuaternion& rotation, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer = PhysicsLayer_Common) final;

	void SetGravity(const FVector& g) final;
	FVector GetGravity() final;

	void OnDelete() override;

public:
	void RemoveBody(CJoltPhysicsBody* body);

public:
	JPH::PhysicsSystem physicsSystem;

	JPH::BodyInterface* bodyInterface;

	BPLayerInterfaceImpl broadPhaseInterface;
	ObjVsBPLayerImpl ObjectBpLayerFilter;
	ObjectLayerPairFilterImpl ObjVsObjFilter;
	ThContactListener contactListener;
	ThBodyActivationListener activactionListener;

	TArray<TObjectPtr<CJoltPhysicsBody>> bodies;
};
