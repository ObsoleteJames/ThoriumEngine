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

	Assimp::Importer importer;
	const aiScene* scene = nullptr;
};

class CModelEditor : public CLayer
{
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
	void CompileNode(const aiScene* scene, aiNode* node, SizeType& meshOffset, SizeType& matOffset, SizeType& boneOffset);

	void DrawMeshResources(FMeshFile& m);
	void DrawAiNode(const aiScene* scene, aiNode* node);

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

	float sizeL = 360;
	float sizeR;

	int viewportWidth = 32, viewportHeight = 32;
	float viewportX = 0.f, viewportY = 0.f;
};
