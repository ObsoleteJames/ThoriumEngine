
#include "MgsPawn.h"

#include "Game/Components/CameraComponent.h"

void CMgsPawn::Init()
{
	BaseClass::Init();

	auto cam = AddComponent<CCameraComponent>("Camera");
	cam->SetPosition(FVector(0, 1.6f, 0));
	cam->AttachTo(RootComponent());

	SetRotation(FQuaternion::EulerAngles(FVector(0, 90, 0).Radians()));
}

void CMgsPawn::Update(double dt)
{
	BaseClass::Update(dt);

	float sin = FMath::Sin((float)GetWorld()->CurTime() * 2.f);
	SetPosition(FVector(0, sin, 2));
}
