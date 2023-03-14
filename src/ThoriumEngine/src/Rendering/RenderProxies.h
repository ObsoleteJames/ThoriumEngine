#pragma once

#include "Object/Object.h"
#include "RenderCommands.h"
#include "Resources/ModelAsset.h"

class CRenderScene;
class CCameraProxy;
class CMaterial;

class ENGINE_API CCameraProxy
{
public:
	CCameraProxy() = default;
	virtual ~CCameraProxy() = default;

	virtual void FetchData() = 0;

	inline bool IsEnabled() const { return bEnabled; }

protected:
	FVector position;
	FMatrix view;
	FMatrix projection;

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
	CPrimitiveProxy() = default;
	virtual ~CPrimitiveProxy() = default;

	virtual void FetchData() = 0;
	virtual void ClearFethedData() {}

	virtual void GetDynamicMeshes(FMeshBuilder& out) {}
	virtual void GetStaticMeshes(FMeshBuilder& out) {}

	virtual bool DoFrustumCull(CCameraProxy* cam);

	virtual void UpdateLOD(CCameraProxy* cam) {}

	inline bool UsesLODs() const { return bHasLod; }
	inline bool IsVisible() const { return bVisible; }
	inline bool CastShadows() const { return bCastShadows; }
	inline bool ReceiveShadows() const { return bReceiveShadows; }

	inline const FVector& GetPosition() const { return position; }
	inline const FMatrix& GetTransform() const { return transform; }
	inline const TArray<FMatrix>& GetSkeletonMatrices() const { return skeletonMatrices; }

	inline const FBounds& Bounds() const { return bounds; }

protected:
	FBounds bounds;

	FVector position;
	FMatrix transform;
	TArray<FMatrix> skeletonMatrices;

	bool bHasLod;
	bool bVisible;
	bool bCastShadows;
	bool bReceiveShadows;
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

	FVector position;
	FVector direction;

	FVector color;
	float range;
	float intensity;

	float innerConeAngle;
	float outerConeAngle;

protected:
	bool bEnabled;
	bool bCastShadows;
};
