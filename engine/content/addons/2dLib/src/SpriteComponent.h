#pragma once

#include "Game/Components/SceneComponent.h"
#include "SpriteComponent.generated.h"

class CTexture;
class CPrimitiveProxy;
class CMaterial;

CLASS()
class ENGINE_API CSpriteComponent : public CSceneComponent
{
	GENERATED_BODY()

public:
	CSpriteComponent() = default;

	void Init();

	void SetTexture(CTexture* tex);
	inline CTexture* GetTexture() const { return spriteTexture; }

	void SetMaterial(CMaterial* mat);
	CMaterial* GetFinalMaterial();

public:
	PROPERTY(Editable)
	TObjectPtr<CTexture> spriteTexture;

	PROPERTY(Editable)
	TObjectPtr<CMaterial> material;

	PROPERTY(Editable)
	int zOrder;

private:
	TObjectPtr<CMaterial> defaultMat;
	TObjectPtr<CModelAsset> planeMdl;
	CPrimitiveProxy* proxy;

};
