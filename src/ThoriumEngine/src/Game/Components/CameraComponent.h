#pragma once

#include "SceneComponent.h"
#include "CameraComponent.generated.h"

class IBaseWindow;

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

public:
	CCameraComponent() = default;

	inline const FMatrix& GetViewMatrix() const { return viewMat; }
	inline const FMatrix& GetProjectionMatrix() const { return projectionMat; }

	void CalculateMatrix(float aspectRatio);

	FRay MouseToRay(float mousex, float mousey, IBaseWindow* window);

	inline float FOV() const { return fov; }

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
};
