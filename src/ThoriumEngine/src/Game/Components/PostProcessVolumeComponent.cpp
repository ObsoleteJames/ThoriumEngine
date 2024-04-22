
#include "PostProcessVolumeComponent.h"
#include "Game/World.h"

void CPostProcessVolumeComp::Init()
{
	BaseClass::Init();

	class Proxy : public CPostProcessVolumeProxy
	{
	public:
		Proxy(CPostProcessVolumeComp* c) : comp(c)
		{
		}

		void FetchData() override
		{
			bEnabled = comp->IsVisible();
			ppSettings = comp->settings;

			bGlobal = comp->bGlobal;
			fade = comp->fadeDistance;
			priority = comp->priority;

			rotation = comp->GetWorldRotation();
			bounds = FBounds(comp->GetWorldPosition(), comp->size * comp->GetWorldScale());

			postProcessMaterial = comp->material;
		}

	public:
		CPostProcessVolumeComp* comp;

	};

	if (GetWorld())
	{
		proxy = new Proxy(this);
		GetWorld()->RegisterPPVolume(proxy);
	}
}

void CPostProcessVolumeComp::OnDelete()
{
	BaseClass::OnDelete();
	GetWorld()->UnregisterPPVolume(proxy);
	delete proxy;
}

FBounds CPostProcessVolumeComp::Bounds() const
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
