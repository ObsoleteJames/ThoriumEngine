
#include "CubeMapComponent.h"
#include "Rendering/RenderScene.h"
#include "Game/World.h"

void CCubeMapComponent::Init()
{
	BaseClass::Init();

	class Proxy : public CCubeMapProxy
	{
	public:
		Proxy(CCubeMapComponent* c) : comp(c)
		{
		}

		void FetchData() override
		{
			bEnabled = comp->IsVisible();

			bGlobal = comp->bGlobal;
			blendWidth = comp->blendWidth;

			rotation = comp->GetWorldRotation();
			//bounds = FBounds(comp->GetWorldPosition(), comp->size * comp->GetWorldScale());
			position = comp->GetWorldPosition();
			size = comp->size * comp->GetWorldScale();
		}

	public:
		CCubeMapComponent* comp;

	};

	if (GetWorld())
	{
		proxy = new Proxy(this);
		if (GetWorld()->GetRenderScene())
			GetWorld()->GetRenderScene()->RegisterCubeMap(proxy);
	}
}

void CCubeMapComponent::OnDelete()
{
	BaseClass::OnDelete();
	if (GetWorld()->GetRenderScene())
		GetWorld()->GetRenderScene()->UnregisterCubeMap(proxy);
	delete proxy;
}

FBounds CCubeMapComponent::Bounds() const
{
	FBounds r = FBounds(FVector(), size);

	FQuaternion rot = GetWorldRotation();
	FVector pos = GetWorldPosition();
	FVector scale = GetWorldScale();

	r.extents *= scale;
	r = r.Rotate(rot);
	r.position = r.position * scale + pos;
	return r;
}
