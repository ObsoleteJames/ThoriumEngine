#pragma once

#include "PrimitiveComponent.h"
#include "Resources/ModelAsset.h"
#include "Resources/Material.h"
#include "ModelComponent.generated.h"

class CAnimGraphInstance;
class CAnimationGraph;
class CModelComponentProxy;

CLASS()
class ENGINE_API CModelComponent : public CPrimitiveComponent
{
	GENERATED_BODY()

	friend class CWorld;
	friend class CModelComponentProxy;

public:
	CModelComponent();

	void SetModel(const FString& file);
	void SetModel(TObjectPtr<CModelAsset> model);
	void SetAnimationGraph(CAnimationGraph* animGraph);

	inline TObjectPtr<CModelAsset> GetModel() const { return model; }
	inline const TArray<TObjectPtr<CMaterial>>& GetMaterials() const { return materials; }

	// call this whenever the skeleton transform has updated.
	inline void UpdateSkeletonMatrix() { bUpdateSkeleton = true; }

	// calculates the skeleton matrices.
	void CalculateSkeletonMatrix();

	inline bool CastShadows() const { return bCastShadows; }

	CMaterial* GetMaterial(SizeType slot);

	void SetMaterial(CMaterial* mat, SizeType slot = 0);
	void SetMaterial(const FString& matPath, SizeType slot = 0);

	virtual void Init();
	virtual void OnStart();
	virtual void OnDelete();

	TArray<FMesh> GetVisibleMeshes(uint8 lodLevel = 0);

	inline const FSkeletonInstance& GetSkeleton() const { return skeleton; }

	FTransform GetBoneModelTransform(int bone) const;

protected:
	virtual void Load(FMemStream& in) override;

private:
	FUNCTION()
	void OnModelEdit();

	void SetupPhysics();

private:
	PROPERTY(Editable, Category = Rendering, OnEditFunc = OnModelEdit)
	TObjectPtr<CModelAsset> model;

	PROPERTY(Editable, Category = Rendering)
	TArray<TObjectPtr<CMaterial>> materials;

	PROPERTY(Editable, Category = Rendering)
	bool bCastShadows = true;

	CModelComponentProxy* renderProxy = nullptr;

	PROPERTY()
	TArray<int> activeBodyGroups;

	bool bUpdateSkeleton = false;

	PROPERTY()
	FSkeletonInstance skeleton;

	TArray<FMatrix> boneMatrices; // cache for skeletal animation

	TArray<TObjectPtr<IPhysicsBody>> physBodies;
};
