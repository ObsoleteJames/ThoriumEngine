
#include "PointLightComponent.h"
#include "Game/World.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"

void CPointLightComponent::Init()
{
	BaseClass::Init();

	class CPointLightProxy : public CLightProxy
	{
	public:
		CPointLightProxy(CPointLightComponent* light) : lightComp(light)
		{
			type = CLightProxy::POINT_LIGHT;
		}

		void FetchData() override
		{
			bEnabled = lightComp->IsVisible();
			bCastShadows = lightComp->bCastShadows;

			bakingMode = CLightProxy::BAKE_NONE;

			position = lightComp->GetWorldPosition();
			color = lightComp->color;
			range = lightComp->range;
			intensity = lightComp->intensity;
		}

	public:
		CPointLightComponent* lightComp;
		
	};

	if (GetWorld())
	{
		lightProxy = new CPointLightProxy(this);
		GetWorld()->RegisterLight(lightProxy);
	}
}

void CPointLightComponent::OnDelete()
{
	BaseClass::OnDelete();
	GetWorld()->UnregisterLight(lightProxy);
	delete lightProxy;
}
