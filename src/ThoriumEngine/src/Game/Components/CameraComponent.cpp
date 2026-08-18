
#include "CameraComponent.h"
#include "Rendering/RenderProxies.h"
#include "Game/World.h"
#include "Window.h"

void CCameraComponent::Init()
{
	BaseClass::Init();

	class CCameraCompProxy : public CCameraProxy
	{
	public:
		CCameraCompProxy(CCameraComponent* comp) : cam(comp) {}

		void FetchData() override
		{
			bEnabled = cam->IsVisible();

			position = cam->GetWorldPosition();
			rotation = cam->GetWorldRotation();

			fov = cam->FOV();
			farPlane = cam->farPlane;
			nearPlane = cam->nearPlane;
		}

	public:
		CCameraComponent* cam;
	};

	if (GetWorld())
	{
		camProxy = new CCameraCompProxy(this);
		GetWorld()->RegisterCamera(camProxy);
	}
}

void CCameraComponent::OnDelete()
{
	BaseClass::OnDelete();

	if (camProxy)
	{
		GetWorld()->UnregisterCamera(camProxy);
		delete camProxy;
	}
}

void CCameraComponent::CalculateMatrix(float aspectRatio)
{
	viewMat = FMatrix(1.f);
	viewMat = viewMat.Translate(GetWorldPosition());
	viewMat *= GetWorldRotation();
	//FVector pos = GetWorldPosition();
	//viewMat = FMatrix::LookAt(pos, pos + GetForwardVector(), GetUpVector());
	viewMat = viewMat.Inverse();

	projectionMat = FMatrix::Perspective(fov, aspectRatio, nearPlane, farPlane);
}

FRay CCameraComponent::MouseToRay(float x, float y, IBaseWindow* window)
{
	int w, h;
	window->GetSize(w, h);

	float viewportX = (float)x / w;
	float viewportY = (float)y / h;

	float ndcX = (viewportX * 2.f) - 1.f;
	float ndcY = 1.f - (viewportY * 2.f);

	float fovRad = glm::radians(fov);
	float tanHalfFov = glm::tan(fovRad * 0.5f);
	float aspectRatio = (float)w / h;

	FVector rayDir = GetForwardVector() +
		(GetRightVector() * ndcX * aspectRatio * tanHalfFov) +
		(GetUpVector() * ndcY * tanHalfFov);
	rayDir = rayDir.Normalize();

	return { GetWorldPosition(), rayDir };
}
