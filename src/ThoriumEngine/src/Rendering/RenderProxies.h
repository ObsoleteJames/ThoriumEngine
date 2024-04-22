#pragma once

#include "Object/Object.h"
#include "RenderCommands.h"
#include "Resources/ModelAsset.h"
#include "RenderProxies.generated.h"

class CRenderScene;
class CCameraProxy;
class CMaterial;
class IFrameBuffer;

class ENGINE_API CCameraProxy
{
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

	bool bEnabled;
};

class ENGINE_API FMeshBuilder
{
public:
	struct FRenderMesh
	{
		FMesh mesh;
		TObjectPtr<CMaterial> mat;
		FMatrix transform;
		TArray<FMatrix> skeletonMatrices;
		ERenderPass rp;

		int lightmapId;
		FVector2 lightmapPos;
		FVector2 lightmapScale;
	};

public:
	void DrawLine(const FVector& begin, const FVector& end, const FVector& color = { 255, 255, 255 }, bool bDepthTest = true);
	void DrawCircle(const FVector& pos, float radius = 1.f, const FVector& rot = FVector(), const FVector & color = { 255, 255, 255 }, int vertices = 16, bool bDepthTest = true);
	void DrawMesh(const FMesh& mesh, CMaterial* mat, const FMatrix& transform, const TArray<FMatrix>& skeletonMatrix = TArray<FMatrix>());

	inline const TArray<FRenderMesh>& GetMeshes() const { return meshes; }

protected:
	TArray<FRenderMesh> meshes;

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

	virtual void GetDynamicMeshes(FMeshBuilder& out) {}
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

protected:
	TObjectPtr<CObject> owner;

	FBounds bounds;

	FTransform transform;
	FMatrix matrix;
	TArray<FMatrix> skeletonMatrices;

	bool bHasLod;
	bool bVisible;
	bool bCastShadows;
	bool bReceiveShadows;

	EMoveType moveType;
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

	enum EBakeMode
	{
		BAKE_NONE,
		BAKE_INDIRECT,
		BAKE_DIRECT,
		BAKE_ALL
	};

public:
	CLightProxy() = default;
	virtual ~CLightProxy() = default;

	virtual void FetchData() = 0;

	inline bool Enabled() const { return bEnabled; }
	inline bool CastShadows() const { return bCastShadows; }

public:
	EType type;
	EBakeMode bakingMode = BAKE_NONE;

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

	// Wether this cubemap should also be used as an IBL source.
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
