#pragma once

#include "Asset.h"
#include "Math/Vectors.h"
#include "ModelAsset.generated.h"

class IVertexBuffer;
class IIndexBuffer;
class CMaterial;

struct FVertex
{
	FVector position;
	FVector normal;
	FVector tangent;
	FVector color;
	float uv1[2];
	float uv2[2];

	int bones[4];
	float boneInfluence[4];
};

struct FBounds
{
	FVector min;
	FVector max;
};

struct ENGINE_API FMesh
{
	enum ETopologyType
	{
		TOPOLOGY_TRIANGLES,
		TOPOLOGY_LINES,
		TOPOLOGY_POINTS,
	};

public:
	TObjectPtr<IVertexBuffer> vertexBuffer;
	TObjectPtr<IIndexBuffer> indexBuffer;

	uint32 numIndices;
	uint32 numVertices;
	uint32 materialIndex;

	ETopologyType topologyType = TOPOLOGY_TRIANGLES;
	FString meshName;

	FBounds bounds;

public:
	// This is only used for saving new mesh data.
	SizeType numVertexData;
	FVertex* vertexData;

	SizeType numIndexData;
	uint* indexData;

	SizeType meshDataOffset;
};

struct FBone
{
	FString name;
	uint32 parent;
	FVector position;
	FVector direction;
};

struct FMaterial
{
	FString name;
	WString path;
	TObjectPtr<CMaterial> obj;
};

struct FSkeleton
{
	TArray<FBone> bones;
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

ASSET(Extension = ".thmdl")
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

	FLODGroup LODs[6];
	uint8 numLODs;

	TArray<FBodyGroup> bodyGroups;

	TArray<FMesh> meshes;
	TArray<FMaterial> materials;
	FSkeleton skeleton;
};
