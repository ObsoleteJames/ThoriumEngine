#pragma once

#include "Asset.h"
#include "Math/Vectors.h"
#include "Math/Bounds.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "Physics/ColliderShapes.h"
#include "ModelAsset.generated.h"

class IVertexBuffer;
class IIndexBuffer;
class CMaterial;

struct FMaterial
{
	FString name;
	FString path;
	TObjectPtr<CMaterial> obj;
};

struct FLODGroup
{
	TArray<uint32> meshIndices; // Indices of meshes used by this LOD Group.
	float distanceBias = 1.f;
};

struct FBodyGroupOption
{
	FString name;
	TArray<uint32> meshIndices;
};

struct FBodyGroup
{
	FString name;
	TArray<FBodyGroupOption> options;
};

struct FModelCollider
{
public:
	EShapeType shapeType = SHAPE_BOX;
	float shape[6] = { 0.f };

	uint meshIndex = -1;
	uint attachBone = -1;

	bool bComplex = false;
};

ASSET(Extension = ".thmdl", ImportableAs = ".fbx;.obj;.gltf;.glb")
class ENGINE_API CModelAsset : public CAsset
{
	GENERATED_BODY()

	friend class CModelEditor;
	friend class CModelCompiler;

public:
	CModelAsset() = default;
	virtual ~CModelAsset();

	void OnInit(IBaseFStream* stream) final;
	void Init(const TArray<FMesh>& meshes);
	//void InitFromObj(const FString& objdata);

	void OnSave(IBaseFStream* stream) final;
	
	void OnLoad(IBaseFStream* stream, uint8 lodLevel) final;
	void Unload(uint8 lodLevel) final;

	// Loads vertex data for CPU usage.
	void LoadMeshData();

	// Deletes CPU vertex data.
	void ClearMeshData();

	inline FBounds GetBounds() const { return bounds; }
	FBounds CalculateBounds();

	FBone* GetBone(const FString& name);
	SizeType GetBoneIndex(const FString& name);

	FTransform GetBoneModelTransform(int bone) const;

public:
	int GetLodFromDistance(float distance);

	inline uint8 LodCount() const { return numLODs; }
	inline bool AllowsInstancing() const { return bAllowInstancing; }
	inline void SetAllowInstancing(bool b) { bAllowInstancing = b; }

	static inline constexpr uint8 GetMaxLodCount() { return 6; }

	inline const FLODGroup* GetLODs() const { return LODs; }
	inline const TArray<FBodyGroup>& GetBodyGroups() const { return bodyGroups; }
	inline const TArray<FMesh>& GetMeshes() const { return meshes; }
	inline const TArray<FMaterial>& GetMaterials() const { return materials; }
	inline const TArray<FModelCollider>& GetColliders() const { return colliders; }
	inline const TArray<TArray<FVector>>& GetConvexMeshes() const { return convexMeshes; }
	inline const FSkeleton& GetSkeleton() const { return skeleton; }

private:
	void ClearMeshes();

	void UpdateBoneMatrices();

	inline void _SetLod(uint8 lod) { SetLodLevel(lod, true); }

	virtual uint8 GetFileVersion() const;

protected:
	bool bAllowInstancing;

	FLODGroup LODs[6];
	uint8 numLODs;

	TArray<FBodyGroup> bodyGroups;

	TArray<FModelCollider> colliders;
	TArray<TArray<FVector>> convexMeshes;

	FBounds bounds;
	TArray<FMesh> meshes;
	TArray<FMaterial> materials;
	FSkeleton skeleton;

	TArray<FMesh> collisionMeshes;
	TArray<FMesh> complexCollisionMeshes;
};
