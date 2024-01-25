#pragma once

#include "Layer.h"
#include "EngineCore.h"
#include "Object/Object.h"
#include "Resources/ModelAsset.h"

#include "assimp/Importer.hpp"

class CWorld;
class CModelEntity;
class CRenderScene;
class CCameraProxy;
class CPointLightComponent;

class IFrameBuffer;
class IDepthBuffer;

struct aiNode;
struct aiScene;

struct FMeshFile
{
	FString file;
	FString name;
	FTransform transform;
	FVector rotation; // fuck quaterions man.

	Assimp::Importer importer;
	const aiScene* scene = nullptr;
};

class CModelEditor : public CLayer
{
	typedef void(CModelEditor::* funcSaveCallback)(int result);

public:
	CModelEditor();

	void SetModel(CModelAsset* mdl);
	inline CModelAsset* GetModel() const { return mdl; }

protected:
	void OnUpdate(double dt) override;
	void OnUIRender() override;

	void OnDetach() override;

	void LoadMeshFile(FMeshFile& m);

	void Compile();
	void CompileNode(FMeshFile& file, const aiScene* scene, aiNode* node, SizeType& meshOffset, SizeType& matOffset, SizeType& boneOffset);
	void CompileArmatureNode(FMeshFile& file, const aiScene* scene, aiNode* node, SizeType& meshOffset, SizeType& matOffset, SizeType& boneOffset);

	void DrawMeshResources(FMeshFile& m);
	void DrawAiNode(const aiScene* scene, aiNode* node);

	void OnSaveNewModel(int r);
	void OnSaveExit(int r);
	void OnSaveOpenModel(int r);

	void SaveMdl();
	void Revert();
	
private:
	bool bSaved = true;
	bool bCompiled = true;
	bool bExit = false;

	TObjectPtr<CModelAsset> mdl;
	TObjectPtr<CModelEntity> modelEnt;
	TObjectPtr<CWorld> scene;

	// List of imported meshes
	TArray<FMeshFile> meshFiles;

	CCameraProxy* camera;

	CPointLightComponent* light1;
	
	IFrameBuffer* framebuffer;

	FString openMdlId;
	FString saveMdlId;

	funcSaveCallback saveCallback = nullptr;

	float sizeL = 420;
	float sizeR;

	int viewportWidth = 32, viewportHeight = 32;
	float viewportX = 0.f, viewportY = 0.f;
};
