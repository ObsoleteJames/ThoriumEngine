
#include "JoltPhysicsBody.h"
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>

bool CJoltPhysicsBody::Init(const FPhysicsBodySettings& settings, CJoltPhysicsWorld* w)
{
	world = w;
	JPH::BodyCreationSettings jSettings;
	
	JPH::Vec3 position = JPH::Vec3(settings.transform.position.x, settings.transform.position.y, settings.transform.position.z);
	JPH::Quat rotation = JPH::Quat(settings.transform.rotation.x, settings.transform.rotation.y, settings.transform.rotation.z, settings.transform.rotation.w);
	JPH::EMotionType motion = settings.moveType == PHBM_STATIC ? JPH::EMotionType::Static : (settings.moveType == PHBM_DYNAMIC ? JPH::EMotionType::Dynamic : JPH::EMotionType::Kinematic);

	component = settings.component;
	if (settings.entity)
		SetOwner(settings.entity);

	switch (settings.shapeType)
	{
	case SHAPE_BOX:
	{
		FShapeBox* shapeBox = (FShapeBox*)settings.shapeData;
		FVector size = shapeBox->size * settings.transform.scale;
		JPH::BoxShapeSettings box(JPH::Vec3(size.x / 2, size.y / 2, size.z / 2));
		JPH::ShapeSettings::ShapeResult shapeResult = box.Create();
		JPH::ShapeRefC shapeRef = shapeResult.Get();

		jSettings = JPH::BodyCreationSettings(shapeRef, position, rotation, motion, (int)settings.physicsLayer);
	}
		break;
	case SHAPE_SPHERE:
	{
		FShapeSphere* shapeSphere = (FShapeSphere*)settings.shapeData;
		JPH::SphereShapeSettings sphere(shapeSphere->radius);
		JPH::ShapeSettings::ShapeResult shapeResult = sphere.Create();
		JPH::ShapeRefC shapeRef = shapeResult.Get();

		jSettings = JPH::BodyCreationSettings(shapeRef, position, rotation, motion, (int)settings.physicsLayer);
	}
		break;
	case SHAPE_PLANE:
		return false;
	case SHAPE_CAPSULE:
	{
		FShapeCapsule* shape = (FShapeCapsule*)settings.shapeData;
		JPH::CapsuleShapeSettings capsule(shape->height / 2, shape->radius);
		JPH::ShapeSettings::ShapeResult shapeResult = capsule.Create();
		JPH::ShapeRefC shapeRef = shapeResult.Get();

		jSettings = JPH::BodyCreationSettings(shapeRef, position, rotation, motion, (int)settings.physicsLayer);
	}
		break;
	case SHAPE_MESH:
	{
		FShapeMesh* shape = (FShapeMesh*)settings.shapeData;
		JPH::VertexList vertices;
		JPH::IndexedTriangleList faces;
		for (auto& v : shape->vertices)
			vertices.push_back(JPH::Float3(v.x, v.y, v.z));
			
		for (int i = 0; i < shape->indices.Size() / 3; i++)
		{
			int ind = i * 3;

			faces.push_back(JPH::IndexedTriangle(shape->indices[ind], shape->indices[ind + 1], shape->indices[ind + 2], 0));
		}

		JPH::MeshShapeSettings mesh(vertices, faces);
		JPH::ShapeSettings::ShapeResult shapeResult = mesh.Create();
		JPH::ShapeRefC shapeRef = shapeResult.Get();

		jSettings = JPH::BodyCreationSettings(shapeRef, position, rotation, motion, (int)settings.physicsLayer);
	}
		break;
	case SHAPE_CONVEX_MESH:
	{
		FShapeMesh* shape = (FShapeMesh*)settings.shapeData;
		JPH::Array<JPH::Vec3> vertices;
		for (auto& v : shape->vertices)
			vertices.push_back(JPH::Vec3(v.x, v.y, v.z));

		JPH::ConvexHullShapeSettings mesh(vertices);
		JPH::ShapeSettings::ShapeResult shapeResult = mesh.Create();
		JPH::ShapeRefC shapeRef = shapeResult.Get();

		jSettings = JPH::BodyCreationSettings(shapeRef, position, rotation, motion, (int)settings.physicsLayer);
	}
		break;
	}

	body = world->bodyInterface->CreateBody(jSettings);
	body->SetUserData((SizeType)this);
	bodyId = body->GetID();
	world->bodyInterface->AddBody(body->GetID(), motion == JPH::EMotionType::Dynamic ? JPH::EActivation::Activate : JPH::EActivation::DontActivate);
	return true;
}

void CJoltPhysicsBody::OnDelete()
{
	BaseClass::OnDelete();

	world->RemoveBody(this);
}

void CJoltPhysicsBody::SetPosition(const FVector& p)
{
	JPH::RVec3 pos(p.x, p.y, p.z);
	world->bodyInterface->SetPosition(bodyId, pos, JPH::EActivation::Activate);
}

FVector CJoltPhysicsBody::GetPosition()
{
	auto p = body->GetPosition();
	return FVector(p.GetX(), p.GetY(), p.GetZ());
}

FVector CJoltPhysicsBody::GetScale()
{
	return FVector();
}

void CJoltPhysicsBody::SetScale(const FVector& s)
{

}

void CJoltPhysicsBody::SetRotation(const FQuaternion& r)
{
	JPH::Quat rot(r.x, r.y, r.z, r.w);
	world->bodyInterface->SetRotation(bodyId, rot, JPH::EActivation::Activate);
}

FQuaternion CJoltPhysicsBody::GetRotation()
{
	auto p = body->GetRotation();
	return FQuaternion(p.GetX(), p.GetY(), p.GetZ(), p.GetW());
}

void CJoltPhysicsBody::SetVelocity(const FVector& p)
{
	JPH::RVec3 v(p.x, p.y, p.z);
	world->bodyInterface->SetLinearVelocity(bodyId, v);
}

FVector CJoltPhysicsBody::GetVelocity()
{
	auto p = body->GetLinearVelocity();
	return FVector(p.GetX(), p.GetY(), p.GetZ());
}

FVector CJoltPhysicsBody::GetVelocity(const FVector& p)
{
	JPH::RVec3 pos(p.x, p.y, p.z);
	auto v = body->GetPointVelocity(pos);
	return FVector(v.GetX(), v.GetY(), v.GetZ());
}

void CJoltPhysicsBody::SetAngularVelocity(const FVector& p)
{
	JPH::Vec3 v(p.x, p.y, p.z);
	world->bodyInterface->SetAngularVelocity(bodyId, v);
}

FVector CJoltPhysicsBody::GetAngularVelocity()
{
	auto p = body->GetAngularVelocity();
	return FVector(p.GetX(), p.GetY(), p.GetZ());
}

bool CJoltPhysicsBody::IsAwake()
{
	return body->IsActive();
}

void CJoltPhysicsBody::Wake()
{
	world->bodyInterface->ActivateBody(bodyId);
}

void CJoltPhysicsBody::AddForce(const FVector& p, const FVector& point /*= FVector::zero*/)
{
	JPH::RVec3 v(p.x, p.y, p.z);

	if (FVector::zero == point)
		world->bodyInterface->AddForce(bodyId, v);
	else
		world->bodyInterface->AddForce(bodyId, v, JPH::RVec3Arg(point.x, point.y, point.z));
}

void CJoltPhysicsBody::AddTorque(const FVector& v)
{
	world->bodyInterface->AddTorque(bodyId, JPH::Vec3Arg(v.x, v.y, v.z));
}

void CJoltPhysicsBody::AddImpulse(const FVector& v, const FVector& point /*= FVector::zero*/)
{
	if (FVector::zero == point)
		world->bodyInterface->AddImpulse(bodyId, JPH::Vec3Arg(v.x, v.y, v.z));
	else
		world->bodyInterface->AddImpulse(bodyId, JPH::Vec3Arg(v.x, v.y, v.z), JPH::RVec3Arg(point.x, point.y, point.z));
}
