
#include "MgsPawn.h"

#include "Game/PlayerController.h"
#include "Game/Input/InputManager.h"
#include "Game/Components/CameraComponent.h"

#include "Console.h"

void CMgsPawn::Init()
{
	BaseClass::Init();

	cam = AddComponent<CCameraComponent>("Camera");
	cam->SetPosition(FVector(0, 1.6f, 0));
	cam->AttachTo(RootComponent());
	cam->nearPlane = 0.025f;

	SetPosition({ 0, 0, 2 });
	look.y = 180.f;
}

void CMgsPawn::Update(double dt)
{
	BaseClass::Update(dt);

	//FVector pos = RootComponent()->GetPosition();
	//float sin = FMath::Sin((float)GetWorld()->CurTime() * 2.f);
	//SetPosition(FVector(pos.x, sin, pos.z));

	FQuaternion camRot = FQuaternion::EulerAngles(look.Radians());
	cam->SetRotation(camRot);
}

void CMgsPawn::SetupInput(CInputManager* inputManager)
{
	inputManager->BindAction("Interact", IE_PRESS, this, &CMgsPawn::Tes);

	inputManager->BindAxis("LookUp", this, &CMgsPawn::LookY);
	inputManager->BindAxis("LookRight", this, &CMgsPawn::LookX);

	inputManager->BindAxis("MoveForward", this, &CMgsPawn::MoveForward);
	inputManager->BindAxis("MoveRight", this, &CMgsPawn::MoveStrafe);
}

void CMgsPawn::Tes()
{
	float rng = FMath::Clamp((float)FMath::Random(60, 120), 60.f, 120.f);
	cam->fov = rng;
}

void CMgsPawn::LookX(float v)
{
	look.y += v / 10.f;
}

void CMgsPawn::LookY(float v)
{
	look.x += v / 10.f;
}

void CMgsPawn::MoveForward(float v)
{
	if (v == 0.f)
		return;

	FVector pos = GetWorldPosition();
	pos += cam->GetForwardVector() * (v / 100.f);

	SetPosition(pos);
}

void CMgsPawn::MoveStrafe(float v)
{
	if (v == 0.f)
		return;

	FVector pos = GetWorldPosition();
	pos += cam->GetRightVector() * (v / 100.f);

	SetPosition(pos);
}
