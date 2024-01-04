#pragma once

#include "Asset.h"
#include "Math/Vectors.h"
#include "Math/Bounds.h"
#include "Mesh.h"
#include "Skeleton.h"
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
	float distanceBias;
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

ASSET(Extension = ".thmdl", ImportableAs = ".fbx;.obj;.gltf")
class ENGINE_API CModelAsset : public CAsset
{
	GENERATED_BODY()

	friend class CModelCreator;

public:
	CModelAsset() = default;
	virtual ~CModelAsset();

	void Init() final;
	void Init(const TArray<FMesh>& meshes);
	//void InitFromObj(const FString& objdata);

	void Save() final;
	
	void Load(uint8 lodLevel) final;
	void Unload(uint8 lodLevel) final;

	// Loads vertex data for CPU usage.
	void LoadMeshData();

	// Deletes CPU vertex data.
	void ClearMeshData();

	inline FBounds GetBounds() const { return bounds; }
	FBounds CalculateBounds();

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
	inline const FSkeleton& GetSkeleton() const { return skeleton; }

private:
	void ClearMeshes();

protected:
	bool bAllowInstancing;

	uint16 fileVersion;

	FLODGroup LODs[6];
	uint8 numLODs;

	TArray<FBodyGroup> bodyGroups;

	FBounds bounds;
	TArray<FMesh> meshes;
	TArray<FMaterial> materials;
	FSkeleton skeleton;

	TArray<FMesh> collisionMeshes;
	TArray<FMesh> complexCollisionMeshes;
};
