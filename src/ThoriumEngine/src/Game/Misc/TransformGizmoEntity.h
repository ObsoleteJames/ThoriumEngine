#pragma once

#include "Game/Entity.h"
#include "Window.h"
#include "TransformGizmoEntity.generated.h"

class CModelComponent;
class CSceneComponent;
class CMaterial;
class CCameraComponent;

ENUM()
enum EGizmoType
{
	GIZMO_TRANSLATE,
	GIZMO_ROTATE,
	GIZMO_SCALE
};

struct FGizmoState
{
	bool bMouseLeft = false;
	bool bCtrl = false;
	double mouseX = 0.0;
	double mouseY = 0.0;
	FRay mouseRay;
	FVector clickPosition;
	FQuaternion clickRotation;
	FVector clickScale;
};

CLASS(Hidden)
class ENGINE_API CTransformGizmoEntity : public CEntity 
{
	GENERATED_BODY()

public:
	CTransformGizmoEntity() = default;

	void SetTargetObject(CSceneComponent* obj);
	inline CSceneComponent* GetTargetObject() const { return targetObject; }

	inline void SetCamera(CCameraComponent* c) { camera = c; }

	void SetGizmoType(EGizmoType t);

	void SetUseLocalSpace(bool b) { bUseLocalSpace = b; }
	inline bool UsesLocalSpace() const { return bUseLocalSpace; }

	inline void EnableSnapPosition(bool b) { bSnapPosition = b; }
	inline void EnableSnapRotation(bool b) { bSnapRotation = b; }
	inline void EnableSnapScale(bool b) { bSnapScale = b; }

	inline void SetPositionSnap(float f) { if (f > 0.f) positionSnap = f; }
	inline void SetRotationSnap(float f) { if (f > 0.f) rotationSnap = f; }
	inline void SetScaleSnap(float f) { if (f > 0.f) scaleSnap = f; }

protected:
	void Init() override;
	void Update(double dt) override;
	void OnDelete() override;

	void UpdateTranslate();
	void UpdateRotate();
	void UpdateScale();

	FVector CalculateDragPosition();

	void OnMouseButton(EMouseButton btn, EInputAction event, EInputMod i);
	void OnKeyEvent(EKeyCode key, EInputAction event, EInputMod mod);

private:
	EGizmoType type = GIZMO_TRANSLATE;
	bool bSnapPosition:1;
	bool bSnapRotation:1;
	bool bSnapScale:1;
	bool bUseLocalSpace = true;
	bool bDrag = false;
	float positionSnap = 1.0f;
	float rotationSnap = 45.f;
	float scaleSnap = 0.5f;

	int dragGizmo = -1;
	SizeType mouseBtnBind = -1;
	SizeType keyEventBind = -1;

	FVector posOffset;
	float drawScale = 1.0f;
	
	FGizmoState state;
	FGizmoState previousState;

	TObjectPtr<CModelComponent> translateGizmo;
	TObjectPtr<CModelComponent> rotateGizmo;
	TObjectPtr<CModelComponent> scaleGizmo;

	TObjectPtr<CMaterial> gizmoMatX;
	TObjectPtr<CMaterial> gizmoMatY;
	TObjectPtr<CMaterial> gizmoMatZ;
	TObjectPtr<CMaterial> gizmoMatW;

	CCameraComponent* camera;
	TObjectPtr<CSceneComponent> targetObject;
};
