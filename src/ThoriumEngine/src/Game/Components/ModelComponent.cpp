
#include "ModelComponent.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"
#include "Game/World.h"

class CModelComponentProxy : public CPrimitiveProxy
{
public:
	CModelComponentProxy(CModelComponent* mdl) : model(mdl)
	{
		owner = mdl;
	}

	void FetchData() override
	{
		meshes.Clear();
		bVisible = model->IsVisible() && model->GetModel();
		bCastShadows = model->CastShadows();
		if (model->GetWorld()->IsActive() && model->bEditorOnly)
			bVisible = false;
		if (!bVisible)
			return;

		//float distanceFromCamera = FVector::Distance(scene->GetCamera()->GetWorldPosition(), model->GetWorldPosition());
		float distanceFromCamera = 0;
		int lodLevel = model->GetModel()->GetLodFromDistance(distanceFromCamera);
		model->GetModel()->Load(lodLevel);

		transform.position = model->GetWorldPosition();
		transform.rotation = model->GetWorldRotation();
		transform.scale = model->GetWorldScale();
		matrix = FMatrix(1.f).Translate(transform.position).Scale(transform.scale) * transform.rotation;
		meshes = model->GetVisibleMeshes(lodLevel);
		materials.Resize(model->GetMaterials().Size());
		bounds = model->Bounds();
		
		if (model->bUpdateSkeleton)
		{
			model->CalculateSkeletonMatrix();
			model->bUpdateSkeleton = false;
		}

		skeletonMatrices = model->boneMatrices;

		for (SizeType i = 0; i < materials.Size(); i++)
		{
			materials[i] = model->GetMaterial(i);
			if (materials[i])
				materials[i]->Load(lodLevel);
		}
	}

	void GetDynamicMeshes(FMeshBuilder& out) override
	{
		for (auto& m : meshes)
		{
			TObjectPtr<CMaterial> mat = (m.materialIndex < materials.Size() && materials[m.materialIndex]) ? materials[m.materialIndex] : CResourceManager::GetResource<CMaterial>("materials/error.thmat");
			out.DrawMesh(m, mat, matrix, skeletonMatrices);
		}
	}

public:
	CModelComponent* model;

	TArray<TObjectPtr<CMaterial>> materials;
	TArray<FMesh> meshes;
	//FMatrix transform;
	//TArray<FMatrix> skeletonMatrices;

};

CModelComponent::CModelComponent()
{
}

void CModelComponent::SetModel(const FString& file)
{
	TObjectPtr<CModelAsset> mdl = CResourceManager::GetResource<CModelAsset>(file);
	if (!mdl)
		mdl = CResourceManager::GetResource<CModelAsset>("models/error.thmdl");

	SetModel(mdl);
}

void CModelComponent::SetModel(TObjectPtr<CModelAsset> m)
{
	//activeBodyGroups.Clear();
	//materials.Clear();
	bounds = FBounds();

	model = m;

	if (!model)
	{
		activeBodyGroups.Clear();
		materials.Clear();
		return;
	}

	activeBodyGroups.Resize(model->GetBodyGroups().Size());
	for (auto& i : activeBodyGroups)
		i = 0;

	const TArray<FMaterial>& mats = model->GetMaterials();
	materials.Resize(mats.Size());

	bounds = model->CalculateBounds();

	skeleton.bones.Clear();
	skeleton.bones.Resize(model->GetSkeleton().bones.Size());

	//for (auto& matPath : mats)
	//{
	//	TObjectPtr<CMaterial> material = CResourceManager::GetResource<CMaterial>(matPath.path);
	//	//if (!material)
	//	//{
	//	//	material = CreateObject<CMaterial>();
	//	//	material->SetShader("Error");
	//	//}

	//	materials.Add(material);
	//}
}

void CModelComponent::SetAnimationGraph(CAnimationGraph* animGraph)
{

}

void CModelComponent::CalculateSkeletonMatrix()
{
	if (!model)
		return;

	const FSkeleton& sk = model->GetSkeleton();
	boneMatrices.Resize(sk.bones.Size());

	for (SizeType i = 0; i < sk.bones.Size(); i++)
	{
		FMatrix& mat = boneMatrices[i];

		FMatrix local = (FMatrix(1.f).Translate(sk.bones[i].position) * sk.bones[i].rotation) * skeleton.bones[i].ToMatrix();
		FMatrix model = sk.bones[i].parent != -1 ? boneMatrices[sk.bones[i].parent] * local : local;

		mat = model * sk.invModel[i];
	}
}

CMaterial* CModelComponent::GetMaterial(SizeType slot)
{
	if (slot < model->GetMaterials().Size())
	{
		if (slot >= materials.Size() || !materials[slot])
			return model->GetMaterials()[slot].obj;
	}

	if (slot >= materials.Size())
		return nullptr;

	return materials[slot];
}

void CModelComponent::SetMaterial(CMaterial* mat, SizeType slot /*= 0*/)
{
	if (materials.Size() < slot + 1)
		materials.Resize(slot + 1);

	materials[slot] = mat;
}

void CModelComponent::Init()
{
	//FWorldRegisterer::RegisterModelComponent(GetWorld(), this);
	if (GetWorld())
	{
		renderProxy = new CModelComponentProxy(this);
		GetWorld()->RegisterPrimitive(renderProxy);
	}

	BaseClass::Init();
}

void CModelComponent::OnDelete()
{
	//FWorldRegisterer::UnregisterModelComponent(GetWorld(), this);
	if (renderProxy)
	{
		GetWorld()->UnregisterPrimitive(renderProxy);
		delete renderProxy;
	}

	model = nullptr;
	materials.Clear();

	BaseClass::OnDelete();
}

TArray<FMesh> CModelComponent::GetVisibleMeshes(uint8 lodLevel /*= 0*/)
{
	TArray<FMesh> meshes;
	if (!model)
		return meshes;

	if (model->LodCount() > 0)
	{
		const TArray<FBodyGroup>& bodyGroups = model->GetBodyGroups();
		for (int i = 0; i < bodyGroups.Size(); i++)
		{
			const FBodyGroupOption& bgOption = bodyGroups[i].options[activeBodyGroups[i]];
			for (auto& bgMeshI : bgOption.meshIndices)
			{
				bool bIsVisible = false;
				for (auto& mI : model->GetLODs()[lodLevel].meshIndices)
				{
					if (mI == bgMeshI)
					{
						bIsVisible = true;
						break;
					}
				}

				if (bIsVisible)
					meshes.Add((model->GetMeshes()[bgMeshI]));
			}
		}
		if (bodyGroups.Size() == 0)
		{
			for (auto& mi : model->GetLODs()[lodLevel].meshIndices)
				meshes.Add((model->GetMeshes()[mi]));
		}
	}
	else
	{
		const TArray<FBodyGroup>& bodyGroups = model->GetBodyGroups();
		for (int i = 0; i < bodyGroups.Size(); i++)
		{
			const FBodyGroupOption& bgOption = bodyGroups[i].options[activeBodyGroups[i]];
			for (auto& bgMeshI : bgOption.meshIndices)
				meshes.Add((model->GetMeshes()[bgMeshI]));
		}
		if (bodyGroups.Size() == 0)
		{
			for (auto& mesh : model->GetMeshes())
				meshes.Add(mesh);
		}
	}

	return meshes;
}

void CModelComponent::Load(FMemStream& in)
{
	BaseClass::Load(in);

	if (model)
		SetModel(model);
}

void CModelComponent::OnModelEdit()
{
	SetModel(model);
}
