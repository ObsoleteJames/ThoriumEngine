#pragma once

#include "SceneComponent.h"
#include "Rendering/RenderProxies.h"
#include "CubeMapComponent.generated.h"

CLASS(Name = "Cube Map Volume")
class CCubeMapComponent : public CSceneComponent
{
	GENERATED_BODY()

public:
	void Init();
	void OnDelete();

	FBounds Bounds() const override;

public:
	PROPERTY(Editable)
	FVector size = FVector::one;
	
	PROPERTY(Editable)
	bool bGlobal = false;

	PROPERTY(Editable)
	bool bAffectDiffuse = false;

	PROPERTY(Editable)
	bool bRealtime = false;

	PROPERTY(Editable)
	float blendWidth = 0.f;

	PROPERTY(Editable)
	ECubemapResolution resolution = CMR_1024;

private:
	CCubeMapProxy* proxy;
};
