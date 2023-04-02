#pragma once

#include "SceneComponent.h"
#include "Game/World.h"
#include "CameraComponent.generated.h"

class IBaseWindow;
class CCameraProxy;

ENUM()
enum EProjectionType
{
	PT_PERSPECTIVE,
	PT_ORTHOGRAPHIC
};

CLASS()
class ENGINE_API CCameraComponent : public CSceneComponent
{
	GENERATED_BODY()

	friend class CCameraCompProxy;

public:
	CCameraComponent() = default;

	void Init();
	void OnDelete();

	inline const FMatrix& GetViewMatrix() const { return viewMat; }
	inline const FMatrix& GetProjectionMatrix() const { return projectionMat; }

	void CalculateMatrix(float aspectRatio);

	FRay MouseToRay(float mousex, float mousey, IBaseWindow* window);

	inline float FOV() const { return fov; }

	inline void MakePrimary() const { GetWorld()->SetPrimaryCamera(camProxy); }

private:
	PROPERTY(Editable)
	float fov = 70.f;

	PROPERTY(Editable)
	float nearPlane = 0.1f;

	PROPERTY(Editable)
	float farPlane = 10000.f;

	PROPERTY(Editable)
	EProjectionType projection;

	FMatrix viewMat;
	FMatrix projectionMat;

	CCameraProxy* camProxy;
};
