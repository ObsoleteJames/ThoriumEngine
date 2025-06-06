
#include "SunLightComponent.h"
#include "Game/World.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"

void CSunLightComponent::Init()
{
	BaseClass::Init();

	class CSunLightProxy : public CLightProxy
	{
	public:
		CSunLightProxy(CSunLightComponent* light) : lightComp(light)
		{
			type = CLightProxy::DIRECTIONAL_LIGHT;
		}

		void FetchData() override
		{
			bEnabled = lightComp->IsVisible();
			bCastShadows = lightComp->bCastShadows;

			bakingMode = CLightProxy::BAKE_NONE;

			direction = -lightComp->GetForwardVector();
			rotation = lightComp->GetWorldRotation();
			color = lightComp->color;
			intensity = lightComp->intensity;
			shadowBias = lightComp->shadowBias;
		}

	public:
		CSunLightComponent* lightComp;

	};

	if (GetWorld())
	{
		lightProxy = new CSunLightProxy(this);
		GetWorld()->RegisterLight(lightProxy);
	}
}

void CSunLightComponent::OnDelete()
{
	BaseClass::OnDelete();
	GetWorld()->UnregisterLight(lightProxy);
	delete lightProxy;
}
