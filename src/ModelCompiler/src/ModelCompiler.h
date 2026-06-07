#pragma once

#include <Util/Core.h>
#include "Assets/ModelAsset.h"

#ifdef _WIN32
#ifdef MODELCOMPILER_DLL
#define MDLCOMPILER_API __declspec(dllexport)
#else
#define MDLCOMPILER_API __declspec(dllimport)
#endif
#else
#define EDITOR_API
#endif

namespace Assimp 
{ 
	class Importer; 
}

class aiScene;
class aiAnimation;
class aiNode;
class aiBone;

struct MDLCOMPILER_API FMeshFile
{
	FString file;
	FString name;
	FTransform transform;
	FVector rotation; // fuck quaternions man.

	Assimp::Importer* importer = nullptr;
	const aiScene* scene = nullptr;
	bool bLoadFailed = false; // wether the aiScene failed to load.
};

enum EColliderImportType
{
	COLLIDER_OFF,
	COLLIDER_SIMPLE,
	COLLIDER_COMPLEX
};

struct MDLCOMPILER_API FModelCompileSettings
{
	bool bCreateMaterials = false;
	bool bImportTextures = false;

	FString materialsOut;

	// if true, will use all meshes as colliders, does NOT generate convex colliders.
	EColliderImportType bUseMeshesAsCollision = COLLIDER_COMPLEX;
};

struct MDLCOMPILER_API FAnimationImportSettings
{
	FString mod;
	FString path;
};

class MDLCOMPILER_API CModelCompiler
{
public:
	CModelCompiler() = default;

	inline void SetModel(CModelAsset* model) { mdl = model; }

	bool Compile(CModelAsset* mdl, FMeshFile* meshFiles, int numMeshFiles, const FModelCompileSettings& settings = FModelCompileSettings());

	/*
	*	Compile the model using the .meta file, usually located in the sdk_content folder.
	* 
	*   if bFullRecompile is false then only the meshes will be compiled.
	*/
	bool CompileFromCfgFile(CModelAsset* mdl, const FString& file, bool bFullRecompile = true);

	/*
	 *	Generates LOD groups based on names of meshes.
	 *	meshes without a suffix will default to LOD 0.
	 *	returns true if any LOD groups were made.
	 */
	bool GenerateLODGroups(FString suffix = "_LOD");

	bool GenerateConvexCollision();

	void SaveModel(FMeshFile* meshFiles, int numMeshFiles);

	bool ExportAnimation(aiAnimation* anim, const FAnimationImportSettings& settings);

	inline const FString& GetError() const { return error; }

private:
	void CompileNode(FMeshFile& file, const aiScene* scene, aiNode* node, SizeType& meshOffset, SizeType& matOffset, TArray<TPair<int, aiBone*>>& outBones);

	SizeType GetMeshIndex(const FString& name);
	FMaterial* GetMaterial(const FString& name);

private:
	CModelAsset* mdl;

	FString error;
};
