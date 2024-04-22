#pragma once

#include "SceneComponent.h"
#include "Rendering/PostProcessing.h"
#include "PostProcessVolumeComponent.generated.h"

CLASS(Name = "Post Process Volume")
class CPostProcessVolumeComp : public CSceneComponent
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
	float fadeDistance = 0.f;

	PROPERTY(Editable)
	int priority = 0;

	PROPERTY(Editable)
	FPostProcessSettings settings;

	PROPERTY(Editable)
	TObjectPtr<CMaterial> material = nullptr;

private:
	CPostProcessVolumeProxy* proxy;
};
