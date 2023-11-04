#pragma once

#include "EngineCore.h"
#include "Game/Components/SceneComponent.h"
#include "Math/Vectors.h"
#include "LightComponent.generated.h"

class IFrameBuffer;

enum ELightType
{
	LIGHT_DIRECTIONAL,
	LIGHT_SPOT,
	LIGHT_OMNI,
	LIGHT_RECT
};

CLASS(Abstract)
class ENGINE_API ILightComponent : public CSceneComponent
{
	GENERATED_BODY()

	friend class IRenderer;

public:
	inline float GetIntensity() const { return intensity; }

	inline bool CastShadows() const { return bCastShadows; }
	inline void SetCastShadows(bool b) { bCastShadows = b; }

	inline bool IsSunlight() const { return bIsSunlight; }

	virtual void Init();
	virtual void OnDelete();

protected:
	ELightType type;
	float intensity;

	bool bIsSunlight = false;
	bool bCastShadows;
};
