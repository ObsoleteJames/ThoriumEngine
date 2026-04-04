#pragma once

#include "EngineCore.h"
#include "Object/Enum.h"
#include "Rendering/RenderLayer.h"
#include "Game/Components/SceneComponent.h"
#include "BillboardComponent.generated.h"

class CPrimitiveProxy;
class CTexture;
class CMaterial;

CLASS()
class ENGINE_API CBillboardComponent : public CSceneComponent
{
	GENERATED_BODY()

	friend class CBillboardPrimitiveProxy;

public:
	virtual ~CBillboardComponent() = default;

	void Init();
	void OnDelete();

	FBounds Bounds() const override;

	void SetSprite(CTexture* tex);

public:
	PROPERTY(Editable)
	TObjectPtr<CTexture> sprite;

	PROPERTY(Editable)
	TEnumInt<ERenderLayer, uint16> renderLayer = R_LAYER_DEFAULT;

private:
	TObjectPtr<CMaterial> mat;
	CPrimitiveProxy* renderProxy;

};
