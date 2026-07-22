#pragma once

#include "Object/Object.h"
#include "RenderCommands.h"
#include "Assets/ModelAsset.h"
#include "RenderLayer.h"
#include "RenderProxies.generated.h"

class CRenderScene;
class CCameraProxy;
class CMaterial;
class IFrameBuffer;

class ENGINE_API CCameraProxy
{
public:
	enum EViewMode
	{
		VIEW_LIT, // standard fully shaded view.
		VIEW_UNLIT, // render diffuse colour with basic shading.
		VIEW_DIFFUSE, // render only diffuse colour.
		VIEW_WIREFRAME,
		VIEW_NORMAL, // render normal map.
		VIEW_REFLECTIONS // reflections only
	};

public:
	CCameraProxy() = default;
	virtual ~CCameraProxy() = default;

	/**
	 * Fetch all required data for this camera to be used in the render thread.
	 */
	virtual void FetchData() {}

	/**
	 * Calulate the camera matrices.
	 * May be called within the render thread.
	 */
	virtual void CalculateMatrix(float aspectRatio);

	inline FVector GetForwardVector() const { return rotation.Rotate({ 0.f, 0.f, 1.f }); }
	inline FVector GetRightVector() const { return rotation.Rotate({ 1.f, 0.f, 0.f }); }
	inline FVector GetUpVector() const { return rotation.Rotate({ 0.f, 1.f, 0.f }); }

	FVector2 WorldSpaceToScreenPos(const FVector& position, const FVector2& screenSize);

public:
	// the cameras render target, if null the scene's render target will be used.
	IFrameBuffer* renderTarget = nullptr;

	FVector position;
	FQuaternion rotation;
	FMatrix view;
	FMatrix projection;

	bool bOrthographic = false;
	float fov = 70.f;
	float nearPlane = 0.1f;
	float farPlane = 10000.f;

	ERenderLayer layers = R_LAYER_DEFAULT;
	EViewMode viewMode = VIEW_LIT;

	bool bDrawWireframe = false;

	bool bEnabled;
};

class ENGINE_API FMeshBuilder
{
public:
	struct FRenderMesh
	{
		FMesh mesh;
		CMaterial* mat;
		FMatrix transform;
		//TArray<FMatrix> skeletonMatrices;
		FMatrix* skeletonMatrices;
		SizeType skeletonMatricesSize;
		ERenderPass rp;

		int lightmapId = -1;
		FVector2 lightmapPos;
		FVector2 lightmapScale;
	};

public:
	FMeshBuilder(TArray<FRenderMesh>* output);

	void DrawLine(const FVector& begin, const FVector& end, const FVector& color = { 255, 255, 255 }, bool bDepthTest = true);
	void DrawCircle(const FVector& pos, float radius = 1.f, const FVector& rot = FVector(), const FVector & color = { 255, 255, 255 }, int vertices = 16, bool bDepthTest = true);
	void DrawSkinnedMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform, const TArray<FMatrix>& skeletonMatrix);
	void DrawMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform);
	void DrawMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform, int lightmapId, const FVector2& lightmapPos, const FVector2& lightmapScale);

	inline const TArray<FRenderMesh>& GetMeshes() const { return *meshes; }

protected:
	TArray<FRenderMesh>* meshes;

};

class ENGINE_API CPrimitiveProxy
{
public:
	enum EMoveType
	{
		STATIC = 1,
		DYNAMIC = 1 << 1,
	};

public:
	CPrimitiveProxy() = default;
	virtual ~CPrimitiveProxy() = default;

	virtual void FetchData() = 0;
	virtual void ClearFethedData() {}

	virtual void GetSkinnedMeshes(FMeshBuilder& out) {}
	virtual void GetStaticMeshes(FMeshBuilder& out) {}

	virtual bool DoFrustumCull(const FMatrix& projection);

	virtual void UpdateLOD(CCameraProxy* cam) {}

	inline bool IsStatic() const { return moveType & EMoveType::STATIC; }
	inline bool IsDynamic() const { return moveType & EMoveType::DYNAMIC; }

	inline bool UsesLODs() const { return bHasLod; }
	inline bool IsVisible() const { return bVisible; }
	inline bool CastShadows() const { return bCastShadows; }
	inline bool ReceiveShadows() const { return bReceiveShadows; }

	inline const FVector& GetPosition() const { return transform.position; }
	inline const FTransform& GetTransform() const { return transform; }
	inline const FMatrix& GetMatrix() const { return matrix; }
	inline const TArray<FMatrix>& GetSkeletonMatrices() const { return skeletonMatrices; }

	inline const FBounds& Bounds() const { return bounds; }

	inline TObjectPtr<CObject> GetOwner() const { return owner; }

	inline ERenderLayer GetLayers() const { return layers; }

protected:
	TObjectPtr<CObject> owner;

	FBounds bounds;

	FTransform transform;
	FMatrix matrix;
	TArray<FMatrix> skeletonMatrices;

	ERenderLayer layers = R_LAYER_DEFAULT;

	bool bHasLod;
	bool bVisible;
	bool bCastShadows;
	bool bReceiveShadows;

	EMoveType moveType;
};

ENUM()
enum ELightBakeMode
{
	LIGHT_BAKE_NONE		META(Name = "No Baking"),
	LIGHT_BAKE_INDIRECT	META(Name = "Indirect Only"),
	LIGHT_BAKE_DIRECT	META(Name = "Direct Only"),
	LIGHT_BAKE_ALL		META(Name = "Direct & Indirect")
};

class ENGINE_API CLightProxy
{
	friend class IRenderer;

public:
	enum EType
	{
		POINT_LIGHT = 1,
		SPOT_LIGHT,
		DIRECTIONAL_LIGHT
	};

public:
	CLightProxy() = default;
	virtual ~CLightProxy() = default;

	virtual void FetchData() = 0;

	inline bool Enabled() const { return bEnabled; }
	inline bool CastShadows() const { return bCastShadows; }

public:
	EType type;
	ELightBakeMode bakingMode = LIGHT_BAKE_NONE;

	FVector position;
	FVector direction;
	FQuaternion rotation;

	FVector color;
	float range;
	float intensity;

	float innerConeAngle;
	float outerConeAngle;

	float shadowBias;

	int shadowIndex = -1;

protected:
	bool bEnabled;
	bool bCastShadows;
};

ENUM()
enum ECubemapResolution
{
	CMR_128 = 128 META(Name = "128x128"),
	CMR_256 = 256 META(Name = "256x256"),
	CMR_512 = 512 META(Name = "512x512"),
	CMR_1024 = 1024 META(Name = "1024x1024"),
	CMR_2048 = 2048 META(Name = "2048x2048")
};

class ITextureCube;

class ENGINE_API CCubeMapProxy
{
	friend class IRenderer;

public:
	CCubeMapProxy() = default;
	virtual ~CCubeMapProxy() = default;

	virtual void FetchData() = 0;

public:
	// Wether this cubemap affects everything.
	bool bGlobal;

	// Wether this cubemap should also be used as an ambient diffuse lighting source.
	bool bAffectDiffuse;

	// Wether this cubemap re-renders at runtime.
	bool bRealtime;

	bool bEnabled;

	float blendWidth;

	FVector position;
	FVector size;
	FQuaternion rotation;

	ECubemapResolution resolution;

	ITextureCube* tex;
};
