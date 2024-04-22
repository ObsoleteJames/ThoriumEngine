
#include "JoltPhysicsWorld.h"
#include "JoltPhysicsBody.h"

#include "Jolt/Physics/Collision/RayCast.h"
#include "Jolt/Physics/Collision/CastResult.h"
#include "Jolt/Physics/Collision/ShapeCast.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"

#include "Game/Components/PrimitiveComponent.h"

int CJoltPhysicsWorld::Init()
{
	physicsSystem.Init(65536, 0, 65536, 1024 * 10, broadPhaseInterface, ObjectBpLayerFilter, ObjVsObjFilter);

	physicsSystem.SetBodyActivationListener(&activactionListener);
	physicsSystem.SetContactListener(&contactListener);

	bodyInterface = &physicsSystem.GetBodyInterface();

	return 1;
}

void CJoltPhysicsWorld::Start()
{
	physicsSystem.OptimizeBroadPhase();
}

void CJoltPhysicsWorld::Update(double timestep)
{
	physicsSystem.Update(timestep, FMath::Max(1, (int)std::round((1 / timestep) / 60)), GetJoltPhysicsApi()->tempAllocator, GetJoltPhysicsApi()->jobSystem);
}

void CJoltPhysicsWorld::ResolveCollisions()
{

}

IPhysicsBody* CJoltPhysicsWorld::CreateBody(const FPhysicsBodySettings& settings)
{
	CJoltPhysicsBody* body = CreateObject<CJoltPhysicsBody>();
	if (!body->Init(settings, this))
	{
		body->Delete();
		return nullptr;
	}

	bodies.Add(body);
	return body;
}

class FJoltBroadPhaseFilter : public JPH::BroadPhaseLayerFilter
{
public:
	FJoltBroadPhaseFilter(EPhysicsLayerFlag l) : layers(l) {}

	virtual bool ShouldCollide(JPH::BroadPhaseLayer inLayer)
	{
		if (inLayer.GetValue() == (int)EPhysicsLayer::STATIC && (layers & PhysicsLayer_Static))
			return true;
		if (inLayer.GetValue() == (int)EPhysicsLayer::DYNAMIC && (layers & PhysicsLayer_Dynamic))
			return true;
		if (inLayer.GetValue() == (int)EPhysicsLayer::COMPLEX && (layers & PhysicsLayer_Complex))
			return true;
		if (inLayer.GetValue() == (int)EPhysicsLayer::TRIGGER && (layers & PhysicsLayer_Trigger))
			return true;

		return false;
	}

	EPhysicsLayerFlag layers;
};

bool CJoltPhysicsWorld::CastRay(const FRay& ray, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer /*= PhysicsLayer_Common*/, float maxDistance /*= FLT_MAX*/)
{
	const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

	JPH::RRayCast _r(FVECTOR_TO_JPH(ray.origin), FVECTOR_TO_JPH(ray.direction));
	JPH::RayCastResult result;

	bool r = query.CastRay(_r, result, FJoltBroadPhaseFilter(physicsLayer));
	if (!r)
		return false;

	auto* body = (CJoltPhysicsBody*)bodyInterface->GetUserData(result.mBodyID);
	outHit.body = body;
	outHit.component = Cast<CPrimitiveComponent>(body->GetEntityComponent());
	outHit.entity = body->GetEntity();
	outHit.position = JPH_TO_FVECTOR(_r.GetPointOnRay(result.mFraction));
	
	auto* shape = bodyInterface->GetShape(result.mBodyID).GetPtr();

	outHit.normal= JPH_TO_FVECTOR(shape->GetSurfaceNormal(result.mSubShapeID2, _r.GetPointOnRay(result.mFraction)));
	outHit.distance = FVector::Distance(ray.origin, outHit.position);
	return true;
}

class FJoltCastShapeCollector : public JPH::CastShapeCollector
{
public:
	FJoltCastShapeCollector(CJoltPhysicsWorld* w) : world(w) {}

	void AddHit(const JPH::CastShapeCollector::ResultType& hit) final
	{
		FHitInfo outHit;

		auto* body = (CJoltPhysicsBody*)world->bodyInterface->GetUserData(hit.mBodyID2);
		outHit.body = body;
		outHit.component = Cast<CPrimitiveComponent>(body->GetEntityComponent());
		outHit.entity = body->GetEntity();
		outHit.position = JPH_TO_FVECTOR(hit.mContactPointOn2);
		outHit.normal = JPH_TO_FVECTOR(hit.mPenetrationAxis.Normalized());

		hits.Add(outHit);
	}

	CJoltPhysicsWorld* world;
	TArray<FHitInfo> hits;
};

bool CJoltPhysicsWorld::CastBox(const FVector& center, const FVector& size, const FQuaternion& rotation, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer /*= PhysicsLayer_Common*/, float maxDistance /*= FLT_MAX*/)
{
	const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

	JPH::Shape* shape = new JPH::BoxShape(FVECTOR_TO_JPH(size) / 2.f);

	FMatrix transform = FTransform(center, rotation).ToMatrix();

	JPH::RShapeCast cast((const JPH::Shape*)shape, JPH::Vec3Arg(1, 1, 1), *(JPH::Mat44*)&transform, FVECTOR_TO_JPH(direction));
	FJoltCastShapeCollector collector(this);

	query.CastShape(cast, JPH::ShapeCastSettings(), JPH::Vec3(), collector, FJoltBroadPhaseFilter(physicsLayer));

	delete shape;

	if (collector.hits.Size() == 0)
		return false;

	FBounds shapeBounds(center, size / 2.f);

	//for (auto& hit : collector.hits)
	//	hit.distance = FVector::Distance(hit.position, shapeBounds.Clamp(hit.position));

	outHit = collector.hits[0];
	outHit.distance = FVector::Distance(outHit.position, shapeBounds.Clamp(outHit.position));
	//for (auto& hit : collector.hits)
	//	if (hit.distance < outHit.distance)
	//		outHit = hit;

	return true;
}

bool CJoltPhysicsWorld::CastSphere(const FVector& center, float radius, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer /*= PhysicsLayer_Common*/, float maxDistance /*= FLT_MAX*/)
{
	const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

	JPH::Shape* shape = new JPH::SphereShape(radius);

	FMatrix transform = FTransform(center).ToMatrix();

	JPH::RShapeCast cast((const JPH::Shape*)shape, JPH::Vec3Arg(1, 1, 1), *(JPH::Mat44*)&transform, FVECTOR_TO_JPH(direction));
	FJoltCastShapeCollector collector(this);

	query.CastShape(cast, JPH::ShapeCastSettings(), JPH::Vec3(), collector, FJoltBroadPhaseFilter(physicsLayer));

	delete shape;

	if (collector.hits.Size() == 0)
		return false;

	for (auto& hit : collector.hits)
		hit.distance = FVector::Distance(center, hit.position) - radius;

	outHit = collector.hits[0];
	for (auto& hit : collector.hits)
		if (hit.distance < outHit.distance)
			outHit = hit;

	return true;
}

bool CJoltPhysicsWorld::CastCapsule(const FVector& center, float radius, float height, const FQuaternion& rotation, const FVector& direction, FHitInfo& outHit, EPhysicsLayerFlag physicsLayer /*= PhysicsLayer_Common*/, float maxDistance /*= FLT_MAX*/)
{
	const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

	JPH::Shape* shape = new JPH::CapsuleShape(height, radius);

	FMatrix transform = FTransform(center, rotation).ToMatrix();

	JPH::RShapeCast cast((const JPH::Shape*)shape, JPH::Vec3Arg(1, 1, 1), *(JPH::Mat44*)&transform, FVECTOR_TO_JPH(direction));
	FJoltCastShapeCollector collector(this);

	query.CastShape(cast, JPH::ShapeCastSettings(), JPH::Vec3(), collector, FJoltBroadPhaseFilter(physicsLayer));

	delete shape;

	if (collector.hits.Size() == 0)
		return false;

	outHit = collector.hits[0];
	outHit.distance = FVector::Distance(center, outHit.position);
	return true;
}

class FJoltCollideShapeCollector : public JPH::CollideShapeCollector
{
public:
	FJoltCollideShapeCollector(CJoltPhysicsWorld* w, TArray<IPhysicsBody*>& out) : world(w), hits(out) {}

	void AddHit(const JPH::CollideShapeCollector::ResultType& hit) final
	{
		auto* body = (CJoltPhysicsBody*)world->bodyInterface->GetUserData(hit.mBodyID2);
		hits.Add(body);
	}

	CJoltPhysicsWorld* world;
	TArray<IPhysicsBody*>& hits;
};

bool CJoltPhysicsWorld::OverlapBox(const FVector& center, const FVector& size, const FQuaternion& rotation, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer /*= PhysicsLayer_Common*/)
{
	const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

	JPH::Shape* shape = new JPH::BoxShape(FVECTOR_TO_JPH(size) / 2.f);

	FMatrix transform = FTransform(center, rotation).ToMatrix();

	FJoltCollideShapeCollector collector(this, outHit);

	query.CollideShape(shape, JPH::Vec3(), *(JPH::Mat44*)&transform, JPH::CollideShapeSettings(), JPH::Vec3(), collector, FJoltBroadPhaseFilter(physicsLayer));

	delete shape;

	if (collector.hits.Size() == 0)
		return false;
	return true;
}

bool CJoltPhysicsWorld::OverlapSphere(const FVector& center, float radius, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer /*= PhysicsLayer_Common*/)
{
	const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

	JPH::Shape* shape = new JPH::SphereShape(radius);

	FMatrix transform = FTransform(center).ToMatrix();

	FJoltCollideShapeCollector collector(this, outHit);

	query.CollideShape(shape, JPH::Vec3(), *(JPH::Mat44*)&transform, JPH::CollideShapeSettings(), JPH::Vec3(), collector, FJoltBroadPhaseFilter(physicsLayer));

	delete shape;

	if (collector.hits.Size() == 0)
		return false;
	return true;
}

bool CJoltPhysicsWorld::OverlapCapsule(const FVector& center, float radius, float height, const FQuaternion& rotation, TArray<IPhysicsBody*>& outHit, EPhysicsLayerFlag physicsLayer /*= PhysicsLayer_Common*/)
{
	const JPH::NarrowPhaseQuery& query = physicsSystem.GetNarrowPhaseQuery();

	JPH::Shape* shape = new JPH::CapsuleShape(height, radius);

	FMatrix transform = FTransform(center, rotation).ToMatrix();

	FJoltCollideShapeCollector collector(this, outHit);

	query.CollideShape(shape, JPH::Vec3(), *(JPH::Mat44*)&transform, JPH::CollideShapeSettings(), JPH::Vec3(), collector, FJoltBroadPhaseFilter(physicsLayer));

	delete shape;

	if (collector.hits.Size() == 0)
		return false;
	return true;
}

void CJoltPhysicsWorld::SetGravity(const FVector& g)
{
	physicsSystem.SetGravity(JPH::Vec3Arg(g.x, g.y, g.z));
}

FVector CJoltPhysicsWorld::GetGravity()
{
	JPH::Vec3 g = physicsSystem.GetGravity();

	return JPH_TO_FVECTOR(g);
}

void CJoltPhysicsWorld::OnDelete()
{
	BaseClass::OnDelete();

	for (auto& body : bodies)
		body->Delete();

	bodies.Clear();
}

void CJoltPhysicsWorld::RemoveBody(CJoltPhysicsBody* body)
{
	for (auto it = bodies.begin(); it != bodies.end(); it++)
	{
		if (*it == body)
		{
			bodyInterface->RemoveBody(body->bodyId);
			bodyInterface->DestroyBody(body->bodyId);
			bodies.Erase(it);
			break;
		}
	}
}

