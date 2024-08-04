#pragma once

#include "EngineCore.h"
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

private:
	TObjectPtr<CMaterial> mat;
	CPrimitiveProxy* renderProxy;

};
