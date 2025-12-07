#pragma once

#include <Util/Core.h>
#include "Assets/ModelAsset.h"

namespace Assimp 
{ 
	class Importer; 
}

class aiScene;
class aiAnimation;
class aiNode;
class aiBone;

struct FMeshFile
{
	FString file;
	FString name;
	FTransform transform;
	FVector rotation; // fuck quaternions man.

	Assimp::Importer* importer;
	const aiScene* scene = nullptr;
	bool bLoadFailed = false; // wether the aiScene failed to load.
};

enum EColliderImportType
{
	COLLIDER_OFF,
	COLLIDER_SIMPLE,
	COLLIDER_COMPLEX
};

struct FModelCompileSettings
{
	bool bCreateMaterials = false;
	bool bImportTextures = false;

	FString materialsOut;

	// if true, will use all meshes as colliders, does NOT generate convex colliders.
	EColliderImportType bUseMeshesAsCollision = COLLIDER_COMPLEX;
};

struct FAnimationImportSettings
{
	FString mod;
	FString path;
};

class CModelCompiler
{
public:
	CModelCompiler() = default;

	inline void SetModel(CModelAsset* model) { mdl = model; }

	bool Compile(CModelAsset* mdl, FMeshFile* meshFiles, int numMeshFiles, const FModelCompileSettings& settings = FModelCompileSettings());

	/*
	 *	Generates LOD groups based on names of meshes.
	 *	meshes without a suffix will default to LOD 0.
	 *	returns true if any LOD groups were made.
	 */
	bool GenerateLODGroups(FString suffix = "_LOD");

	bool GenerateConvexCollision();

	void SaveModel(FMeshFile* meshFiles, int numMeshFiles);

	bool ExportAnimation(aiAnimation* anim, const FAnimationImportSettings& settings);

private:
	void CompileNode(FMeshFile& file, const aiScene* scene, aiNode* node, SizeType& meshOffset, SizeType& matOffset, TArray<TPair<int, aiBone*>>& outBones);

private:
	CModelAsset* mdl;
};
