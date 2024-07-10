
#include "TransformGizmoEntity.h"
#include "Assets/ModelAsset.h"
#include "Assets/Material.h"
#include "Rendering/Renderer.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Components/ModelComponent.h"

void CTransformGizmoEntity::SetTargetObject(CSceneComponent* obj)
{
	targetObject = obj;
	bIsVisible = targetObject;

	if (targetObject)
		SetWorldPosition(targetObject->GetWorldPosition());
}

void CTransformGizmoEntity::SetGizmoType(EGizmoType t)
{
	type = t;

	switch (type)
	{
	case GIZMO_TRANSLATE:
		translateGizmo->bIsVisible = true;
		rotateGizmo->bIsVisible = false;
		scaleGizmo->bIsVisible = false;
		break;
	case GIZMO_ROTATE:
		translateGizmo->bIsVisible = false;
		rotateGizmo->bIsVisible = true;
		scaleGizmo->bIsVisible = false;
		break;
	case GIZMO_SCALE:
		translateGizmo->bIsVisible = false;
		rotateGizmo->bIsVisible = false;
		scaleGizmo->bIsVisible = true;
		break;
	}
}

void CTransformGizmoEntity::Init()
{
	BaseClass::Init();

	translateGizmo = AddComponent<CSceneComponent>("Translate Gizmo");
	rotateGizmo = AddComponent<CModelComponent>("Rotate Gizmo");
	scaleGizmo = AddComponent<CModelComponent>("Scale Gizmo");

	translateArrowX = AddComponent<CModelComponent>("Translate Arrow X");
	translateArrowY = AddComponent<CModelComponent>("Translate Arrow Y");
	translateArrowZ = AddComponent<CModelComponent>("Translate Arrow Z");

	translateGizmo->AttachTo(rootComponent);
	rotateGizmo->AttachTo(rootComponent);
	scaleGizmo->AttachTo(rootComponent);

	translateArrowX->AttachTo(translateGizmo);
	translateArrowY->AttachTo(translateGizmo);
	translateArrowZ->AttachTo(translateGizmo);

	translateArrowX->SetModel("editor/models/arrow.thmdl");
	translateArrowX->SetPosition(FVector(0.5f, 0, 0));
	translateArrowX->SetRotation(FQuaternion::EulerAngles(FVector(0, -90.f, 0).Radians()));

	translateArrowY->SetModel("editor/models/arrow.thmdl");
	translateArrowY->SetPosition(FVector(0, 0.5f, 0));
	translateArrowY->SetRotation(FQuaternion::EulerAngles(FVector(90.f, 0, 0).Radians()));

	translateArrowZ->SetModel("editor/models/arrow.thmdl");
	translateArrowZ->SetPosition(FVector(0, 0, 0.5f));
	translateArrowZ->SetRotation(FQuaternion::EulerAngles(FVector(-180.f, 0, 0).Radians()));

	//translateGizmo->SetModel(L"editor\\models\\TransformGizmo.thmdl");

	gizmoMatX = CreateObject<CMaterial>();
	gizmoMatX->SetShader("Tools");
	gizmoMatX->SetInt("vType", 3);
	gizmoMatX->SetInt("vInt1", 0);
	gizmoMatX->EnableDepthTest(false);

	gizmoMatY = CreateObject<CMaterial>();
	gizmoMatY->SetShader("Tools");
	gizmoMatY->SetInt("vType", 3);
	gizmoMatY->SetInt("vInt1", 1);
	gizmoMatY->EnableDepthTest(false);

	gizmoMatZ = CreateObject<CMaterial>();
	gizmoMatZ->SetShader("Tools");
	gizmoMatZ->SetInt("vType", 3);
	gizmoMatZ->SetInt("vInt1", 2);
	gizmoMatZ->EnableDepthTest(false);

	translateArrowX->SetMaterial(gizmoMatX, 0);
	translateArrowY->SetMaterial(gizmoMatY, 0);
	translateArrowZ->SetMaterial(gizmoMatZ, 0);

	bIsVisible = false;

	SetGizmoType(GIZMO_TRANSLATE);
}

void CTransformGizmoEntity::Update(double dt)
{
	BaseClass::Update(dt);

	if (!bHasBoundEvents)
	{
		if (auto* wnd = GetWorld()->GetRenderWindow(); wnd)
		{
			wnd->OnMouseButton.Bind(this, &CTransformGizmoEntity::OnMouseButton);
			wnd->OnKeyEvent.Bind(this, &CTransformGizmoEntity::OnKeyEvent);
			bHasBoundEvents = true;
		}
	}

	if (!targetObject || !camera || !GetWorld()->GetRenderWindow())
		return;

	if (bUseLocalSpace)
		SetWorldRotation(targetObject->GetWorldRotation());

	SetWorldPosition(targetObject->GetWorldPosition());

	GetWorld()->GetRenderWindow()->GetMousePos(state.mouseX, state.mouseY);
	state.mouseRay = FRay::MouseToRay(camera, { (float)state.mouseX, (float)state.mouseY }, GetWorld()->GetRenderWindow());

	float dist = FVector::Distance(rootComponent->GetWorldPosition(), camera->position);
	drawScale = FMath::Max(FMath::Tan(camera->fov) * dist * 0.15f, 0.5f);
	SetScale(FVector(drawScale));

	switch (type)
	{
	case GIZMO_TRANSLATE:
		UpdateTranslate();
		break;
	case GIZMO_ROTATE:
		UpdateRotate();
		break;
	case GIZMO_SCALE:
		UpdateScale();
		break;
	}

	previousState = state;
}

void CTransformGizmoEntity::OnDelete()
{
	GetWorld()->GetRenderWindow()->OnMouseButton.RemoveAll(this);
	GetWorld()->GetRenderWindow()->OnKeyEvent.RemoveAll(this);

	BaseClass::OnDelete();
}

void CTransformGizmoEntity::UpdateGizmoPositions()
{
	FVector dirToCam = (GetWorldPosition() - camera->position).Normalize();
	
	float dot = FVector::Dot(rootComponent->GetRightVector(), dirToCam);
	if (dot < 0.f)
	{
		translateArrowX->SetPosition(FVector(0.5f, 0, 0));
		translateArrowX->SetRotation(FQuaternion::EulerAngles(FVector(0, -90.f, 0).Radians()));
	}
	else
	{
		translateArrowX->SetPosition(FVector(-0.5f, 0, 0));
		translateArrowX->SetRotation(FQuaternion::EulerAngles(FVector(0, 90.f, 0).Radians()));
	}

	dot = FVector::Dot(rootComponent->GetUpVector(), dirToCam);
	if (dot < 0.f)
	{
		translateArrowY->SetPosition(FVector(0, 0.5f, 0));
		translateArrowY->SetRotation(FQuaternion::EulerAngles(FVector(90.f, 0, 0).Radians()));
	}
	else
	{
		translateArrowY->SetPosition(FVector(0, -0.5f, 0));
		translateArrowY->SetRotation(FQuaternion::EulerAngles(FVector(-90.f, 0, 0).Radians()));
	}

	dot = FVector::Dot(rootComponent->GetForwardVector(), dirToCam);
	if (dot < 0.f)
	{
		translateArrowZ->SetPosition(FVector(0, 0, 0.5f));
		translateArrowZ->SetRotation(FQuaternion::EulerAngles(FVector(-180.f, 0, 0).Radians()));
	}
	else
	{
		translateArrowZ->SetPosition(FVector(0, 0, -0.5f));
		//translateArrowZ->SetRotation(FQuaternion::EulerAngles(FVector(0, 0, 0).Radians()));
	}
}

void CTransformGizmoEntity::UpdateTranslate()
{
	if (!state.bMouseLeft && previousState.bMouseLeft)
	{
		bDrag = false;
		dragGizmo = -1;
	}
	else if (bDrag)
	{
		bool bSnap = bSnapPosition ^ state.bCtrl;

		FVector pos = CalculateDragPosition();
		if (bSnap)
		{
			const FVector axis[3] = { targetObject->GetRightVector(), targetObject->GetUpVector(), targetObject->GetForwardVector() };

			FVector p = pos * axis[dragGizmo];
			p.x = FMath::Mod(p.x, positionSnap);
			p.y = FMath::Mod(p.y, positionSnap);
			p.z = FMath::Mod(p.z, positionSnap);

			pos -= p;
		}

		targetObject->SetWorldPosition(pos);
		return;
	}

	gizmoMatX->SetFloat("vVar1", 0.0f);
	gizmoMatY->SetFloat("vVar1", 0.0f);
	gizmoMatZ->SetFloat("vVar1", 0.0f);

	bool bHit;
	double hitDist;
	double bestHit = 0.0;

	int8 selectedAxis = -1;

	UpdateGizmoPositions();

	FVector dir = translateArrowX->GetForwardVector();
	FVector center = translateArrowX->GetWorldPosition();

	//center = translateArrowX->GetWorldPosition();

	FMath::RayCylinderIntersection(center, dir, 0.05f * drawScale, 0.7f * drawScale, state.mouseRay.origin, state.mouseRay.direction, bHit, hitDist);
	if (bHit && bestHit < hitDist)
	{
		bestHit = hitDist;
		selectedAxis = 0;
	}

	dir = translateArrowY->GetForwardVector();
	center = translateArrowY->GetWorldPosition();

	FMath::RayCylinderIntersection(center, dir, 0.035 * drawScale, 0.7f * drawScale, state.mouseRay.origin, state.mouseRay.direction, bHit, hitDist);
	if (bHit && bestHit < hitDist)
	{
		bestHit = hitDist;
		selectedAxis = 1;
	}

	dir = translateArrowZ->GetForwardVector();
	center = translateArrowZ->GetWorldPosition();

	FMath::RayCylinderIntersection(center, dir, 0.035 * drawScale, 0.7f * drawScale, state.mouseRay.origin, state.mouseRay.direction, bHit, hitDist);
	if (bHit && bestHit < hitDist)
	{
		bestHit = hitDist;
		selectedAxis = 2;
	}

	if (selectedAxis != -1)
	{
		switch (selectedAxis)
		{
		case 0:
			gizmoMatX->SetFloat("vVar1", 1.0f);
			break;
		case 1:
			gizmoMatY->SetFloat("vVar1", 1.0f);
			break;
		case 2:
			gizmoMatZ->SetFloat("vVar1", 1.0f);
			break;
		}

		if (state.bMouseLeft && !previousState.bMouseLeft)
		{
			bDrag = true;
			dragGizmo = selectedAxis;
			state.clickPosition = targetObject->GetWorldPosition();
			posOffset = FVector();
			posOffset = CalculateDragPosition() - targetObject->GetWorldPosition();
		}
	}
}

void CTransformGizmoEntity::UpdateRotate()
{

}

void CTransformGizmoEntity::UpdateScale()
{

}

FVector CTransformGizmoEntity::CalculateDragPosition()
{
	const FVector axis[3] = { targetObject->GetRightVector(), targetObject->GetUpVector(), targetObject->GetForwardVector() };

	FVector planeTangent = FVector::Cross(axis[dragGizmo], targetObject->GetWorldPosition() - camera->position);
	FVector planeNormal = FVector::Cross(axis[dragGizmo], planeTangent);

	FVector& rayDir = state.mouseRay.direction;

	float denom = FVector::Dot(rayDir, planeNormal);
	if (FMath::Abs(denom) == 0)
		return targetObject->GetWorldPosition();

	float t = FVector::Dot(state.clickPosition - state.mouseRay.origin, planeNormal) / denom;
	if (t < 0)
		return targetObject->GetWorldPosition();

	FVector pos = state.mouseRay.origin + rayDir * t;
	pos = state.clickPosition + axis[dragGizmo] * FVector::Dot(pos - state.clickPosition, axis[dragGizmo]);

	return pos - posOffset;
}

void CTransformGizmoEntity::OnMouseButton(EMouseButton btn, EInputAction event, EInputMod i)
{
	if (btn == EMouseButton::LEFT)
	{
		if (event == IE_PRESS)
			state.bMouseLeft = true;
		else if (event == IE_RELEASE)
			state.bMouseLeft = false;
	}
}

void CTransformGizmoEntity::OnKeyEvent(EKeyCode key, EInputAction event, EInputMod mod)
{
	if (key == EKeyCode::LEFT_CONTROL)
	{
		if (event == IE_PRESS)
			state.bCtrl = true;
		else if (event == IE_RELEASE)
			state.bCtrl = false;
	}
}
