
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

	y = h - y;

	float mouseX = x / ((float)w * 0.5f) - 1.f;
	float mouseY = y / ((float)h * 0.5f) - 1.f;

	FMatrix invVP = (projectionMat * viewMat).Inverse();
	glm::vec4 screenPos = glm::vec4(mouseX, mouseY, 1.f, 1.f);
	glm::vec4 worldPos = (glm::mat4)invVP * screenPos;

	FVector dir = glm::normalize(glm::vec3(worldPos));

	return { GetWorldPosition(), dir };
}
