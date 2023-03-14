
#include "TransformGizmoEntity.h"
#include "Resources/ModelAsset.h"
#include "Resources/Material.h"
#include "Rendering/Renderer.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Components/ModelComponent.h"

void CTransformGizmoEntity::SetTargetObject(CSceneComponent* obj)
{
	targetObject = obj;
	bIsVisible = targetObject;

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

	translateGizmo = AddComponent<CModelComponent>("Translate Gizmo");
	rotateGizmo = AddComponent<CModelComponent>("Rotate Gizmo");
	scaleGizmo = AddComponent<CModelComponent>("Scale Gizmo");

	translateGizmo->AttachTo(rootComponent);
	rotateGizmo->AttachTo(rootComponent);
	scaleGizmo->AttachTo(rootComponent);

	//TArray<FMesh> translateMeshes;
	//translateMeshes.Resize(3);

	//translateMeshes[0].materialIndex = 0;
	//translateMeshes[0].numVertices = 2;
	//translateMeshes[0].topologyType = FMesh::TOPOLOGY_LINES;
	//translateMeshes[0].vertexBuffer = gRenderer->CreateVertexBuffer({ { FVector(0, 0, 0) }, { FVector(1, 0, 0) } });

	//translateMeshes[1].materialIndex = 1;
	//translateMeshes[1].numVertices = 2;
	//translateMeshes[1].topologyType = FMesh::TOPOLOGY_LINES;
	//translateMeshes[1].vertexBuffer = gRenderer->CreateVertexBuffer({ { FVector(0, 0, 0) }, { FVector(0, 1, 0) } });

	//translateMeshes[2].materialIndex = 2;
	//translateMeshes[2].numVertices = 2;
	//translateMeshes[2].topologyType = FMesh::TOPOLOGY_LINES;
	//translateMeshes[2].vertexBuffer = gRenderer->CreateVertexBuffer({ { FVector(0, 0, 0) }, { FVector(0, 0, 1) } });

	//TObjectPtr<CModelAsset> translateModel = CreateObject<CModelAsset>();
	//translateModel->Init(translateMeshes);
	//translateGizmo->SetModel(translateModel);

	translateGizmo->SetModel(L"editor\\models\\TransformGizmo.thmdl");

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

	translateGizmo->SetMaterial(gizmoMatX, 1);
	translateGizmo->SetMaterial(gizmoMatY, 0);
	translateGizmo->SetMaterial(gizmoMatZ, 3);

	bIsVisible = false;

	SetGizmoType(GIZMO_TRANSLATE);
}

void CTransformGizmoEntity::Update(double dt)
{
	BaseClass::Update(dt);

	if (mouseBtnBind == -1)
		if (auto* wnd = GetWorld()->GetRenderWindow(); wnd)
			mouseBtnBind = wnd->OnMouseButton.Bind(this, &CTransformGizmoEntity::OnMouseButton);

	if (keyEventBind == -1)
		if (auto* wnd = GetWorld()->GetRenderWindow(); wnd)
			keyEventBind = wnd->OnKeyEvent.Bind(this, &CTransformGizmoEntity::OnKeyEvent);

	if (!targetObject || !camera || !GetWorld()->GetRenderWindow())
		return;

	if (bUseLocalSpace)
		SetWorldRotation(targetObject->GetWorldRotation());

	SetWorldPosition(targetObject->GetWorldPosition());

	GetWorld()->GetRenderWindow()->GetMousePos(state.mouseX, state.mouseY);
	state.mouseRay = camera->MouseToRay(state.mouseX, state.mouseY, GetWorld()->GetRenderWindow());

	float dist = FVector::Distance(rootComponent->GetWorldPosition(), camera->GetWorldPosition());
	drawScale = FMath::Max(FMath::Tan(camera->FOV()) * dist * 0.15f, 0.5f);
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
	if (mouseBtnBind != -1)
		GetWorld()->GetRenderWindow()->OnMouseButton.Remove(mouseBtnBind);

	BaseClass::OnDelete();
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

	FVector dir = rootComponent->GetRightVector();
	FVector center = rootComponent->GetWorldPosition() + ((dir * 0.5f) * drawScale);

	FMath::RayCylinderIntersection(center, dir, 0.035 * drawScale, 1.0 * drawScale, state.mouseRay.origin, state.mouseRay.direction, bHit, hitDist);
	if (bHit && bestHit < hitDist)
	{
		bestHit = hitDist;
		selectedAxis = 0;
	}

	dir = rootComponent->GetUpVector();
	center = rootComponent->GetWorldPosition() + ((dir * 0.5f) * drawScale);

	FMath::RayCylinderIntersection(center, dir, 0.035 * drawScale, 1.0 * drawScale, state.mouseRay.origin, state.mouseRay.direction, bHit, hitDist);
	if (bHit && bestHit < hitDist)
	{
		bestHit = hitDist;
		selectedAxis = 1;
	}

	dir = rootComponent->GetForwardVector();
	center = rootComponent->GetWorldPosition() + ((dir * 0.5f) * drawScale);

	FMath::RayCylinderIntersection(center, dir, 0.035 * drawScale, 1.0 * drawScale, state.mouseRay.origin, state.mouseRay.direction, bHit, hitDist);
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

	FVector planeTangent = FVector::Cross(axis[dragGizmo], targetObject->GetWorldPosition() - camera->GetWorldPosition());
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
