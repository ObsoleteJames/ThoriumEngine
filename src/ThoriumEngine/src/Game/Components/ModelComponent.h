#pragma once

#include "SceneComponent.h"
#include "Resources/ModelAsset.h"
#include "Resources/Material.h"
#include "ModelComponent.generated.h"

class CAnimGraphInstance;
class CAnimationGraph;
class CModelComponentProxy;

CLASS()
class ENGINE_API CModelComponent : public CSceneComponent
{
	GENERATED_BODY()

	friend class CWorld;

public:
	CModelComponent();

	void SetModel(const WString& file);
	void SetModel(TObjectPtr<CModelAsset> model);
	void SetAnimationGraph(CAnimationGraph* animGraph);

	inline TObjectPtr<CModelAsset> GetModel() const { return model; }
	inline const TArray<TObjectPtr<CMaterial>>& GetMaterials() const { return materials; }

	inline bool CastShadows() const { return bCastShadows; }

	CMaterial* GetMaterial(SizeType slot);

	void SetMaterial(CMaterial* mat, SizeType slot = 0);

	virtual void Init();
	virtual void OnDelete();

	TArray<FMesh> GetVisibleMeshes(uint8 lodLevel = 0);

protected:
	virtual void Load(FMemStream& in) override;

private:
	FUNCTION()
	void OnModelEdit();

private:
	PROPERTY(Editable, Category = Rendering, OnEditFunc = OnModelEdit)
	TObjectPtr<CModelAsset> model;

	PROPERTY(Editable, Category = Rendering)
	TArray<TObjectPtr<CMaterial>> materials;

	PROPERTY(Editable, Category = Rendering)
	bool bCastShadows = true;

	CModelComponentProxy* renderProxy;

	PROPERTY()
	TArray<int> activeBodyGroups;

	FSkeleton skeleton;

};
