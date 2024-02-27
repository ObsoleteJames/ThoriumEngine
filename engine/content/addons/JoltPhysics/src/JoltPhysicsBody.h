#pragma once

#include "JoltPhysicsWorld.h"
#include "Physics/PhysicsBody.h"
#include "JoltPhysicsBody.generated.h"

CLASS()
class JOLTPHYSICS_API CJoltPhysicsBody : public IPhysicsBody
{
	GENERATED_BODY()

public:
	bool Init(const FPhysicsBodySettings& settings, CJoltPhysicsWorld* world);

	void OnDelete() override;

	void SetPosition(const FVector& p) final;
	FVector GetPosition() final;

	FVector GetScale() final;
	void SetScale(const FVector& s) final;

	void SetRotation(const FQuaternion& r) final;
	FQuaternion GetRotation() final;

	void SetVelocity(const FVector& v) final;
	FVector GetVelocity() final;
	FVector GetVelocity(const FVector& point) final;

	void SetAngularVelocity(const FVector& v) final;
	FVector GetAngularVelocity() final;

	bool IsAwake() final;
	void Wake() final;

	void AddForce(const FVector& v, const FVector& point = FVector::zero) final;
	void AddTorque(const FVector& v) final;

	void AddImpulse(const FVector& v, const FVector& point = FVector::zero) final;

public:
	TObjectPtr<CJoltPhysicsWorld> world;

	JPH::Body* body;
	JPH::BodyID bodyId;

};
