
#include "ModelComponent.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"
#include "Game/World.h"
#include "Physics/PhysicsBody.h"
#include "Physics/PhysicsWorld.h"
#include "Assets/Skeleton.h"

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

		CRenderScene* scene = Cast<CModelComponent>(owner)->GetWorld()->GetRenderScene();
		CCameraProxy* camera = scene ? scene->GetPrimaryCamera() : nullptr;

		float distanceFromCamera = FVector::Distance(camera ? camera->position : FVector(), model->GetWorldPosition());
		//float distanceFromCamera = 0;
		int lodLevel = model->GetModel()->GetLodFromDistance(distanceFromCamera);
		model->GetModel()->Load(lodLevel);

		transform.position = model->GetWorldPosition();
		transform.rotation = model->GetWorldRotation();
		transform.scale = model->GetWorldScale();
		matrix = (FMatrix(1.f).Translate(transform.position) * transform.rotation).Scale(transform.scale);
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
			TObjectPtr<CMaterial> mat = (m.materialIndex < materials.Size() && materials[m.materialIndex]) ? materials[m.materialIndex] : CAssetManager::GetAsset<CMaterial>("materials/error.thmat");
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
	TObjectPtr<CModelAsset> mdl = CAssetManager::GetAsset<CModelAsset>(file);
	if (!mdl)
		mdl = CAssetManager::GetAsset<CModelAsset>("models/error.thmdl");

	SetModel(mdl);
}

void CModelComponent::SetModel(TObjectPtr<CModelAsset> m)
{
	//activeBodyGroups.Clear();
	//materials.Clear();
	bounds = FBounds();

	model = m;

	for (auto& p : physBodies)
		p->Delete();

	physBodies.Clear();

	if (physicsBody)
		physicsBody->Delete();
	physicsBody = nullptr;

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

	UpdateSkeletonMatrix();

	if (GetWorld()->IsActive())
		SetupPhysics();

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
	if (!model || model->GetSkeleton().bones.Size() == 0)
		return;

	const FSkeleton& sk = model->GetSkeleton();
	boneMatrices.Resize(sk.bones.Size());

	TArray<FMatrix> local(boneMatrices.Size());
	TArray<FMatrix> model(boneMatrices.Size());

	for (SizeType i = 0; i < sk.bones.Size(); i++)
	{
		local[i] = (FMatrix(1.f).Translate(sk.bones[i].position) * sk.bones[i].rotation) * skeleton.bones[i].ToMatrix();
		//FTransform localTransform(sk.bones[i].position + skeleton.bones[i].position, sk.bones[i].rotation * skeleton.bones[i].rotation);
		//local[i] = localTransform.ToMatrix();
	}

	model[0] = local[0];

	for (SizeType i = 1; i < sk.bones.Size(); i++)
	{
		int parent = sk.bones[i].parent;
		model[i] = model[parent] * local[i];
	}

	for (SizeType i = 0; i < sk.bones.Size(); i++)
	{
		boneMatrices[i] = model[i] * sk.invModel[i];
	}

	//for (SizeType i = 0; i < sk.bones.Size(); i++)
	//{
	//	FMatrix& mat = boneMatrices[i];

	//	FMatrix local = (FMatrix(1.f).Translate(sk.bones[i].position) * sk.bones[i].rotation) * skeleton.bones[i].ToMatrix();
	//	FMatrix model = /*sk.bones[i].parent != -1 ? boneMatrices[sk.bones[i].parent] * local :*/ local;

	//	mat = sk.invModel[i] * model;
	//}
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

void CModelComponent::SetMaterial(const FString& matPath, SizeType slot /*= 0*/)
{
	TObjectPtr<CMaterial> mat = CAssetManager::GetAsset<CMaterial>(matPath);
	if (!mat)
		mat = CAssetManager::GetAsset<CMaterial>("materials/error.thmat");

	SetMaterial(mat, slot);
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

void CModelComponent::OnStart()
{
	SetupPhysics();
}

void CModelComponent::OnDelete()
{
	//FWorldRegisterer::UnregisterModelComponent(GetWorld(), this);
	if (renderProxy)
	{
		GetWorld()->UnregisterPrimitive(renderProxy);
		delete renderProxy;
		renderProxy = nullptr;
	}

	for (auto& p : physBodies)
		p->Delete();
	physBodies.Clear();

	if (physicsBody)
		physicsBody->Delete();
	physicsBody = nullptr;

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

FTransform CModelComponent::GetBoneModelTransform(int bone) const
{
	if (!model)
		return FTransform();

	FTransform r = skeleton.bones[bone] * FTransform(model->GetSkeleton().bones[bone].position, model->GetSkeleton().bones[bone].rotation);

	int pIndex = model->GetSkeleton().bones[bone].parent;
	if (pIndex != -1)
	{
		//FTransform parent = GetBoneModelTransform(skeleton.bones[bone].parent);
		FTransform parent = GetBoneModelTransform(pIndex);
		r = r * parent;
	}

	return r;
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

void CModelComponent::SetupPhysics()
{
	if (GetWorld() && GetWorld()->GetPhysicsWorld())
	{
		auto* physWorld = GetWorld()->GetPhysicsWorld();

		for (auto& coll : model->GetColliders())
		{
			FPhysicsBodySettings bodySettings{};
			bodySettings.component = this;
			bodySettings.entity = GetEntity();
			bodySettings.motionType = GetEntity()->type == ENTITY_STATIC ? PHBM_STATIC : (bStaticBody ? PHBM_KINEMATIC : PHBM_DYNAMIC);
			if (bIsTrigger)
				bodySettings.motionType = PHBM_STATIC;

			// :P
			bodySettings.physicsLayer = bIsTrigger ? EPhysicsLayer::TRIGGER : (coll.bComplex ? EPhysicsLayer::COMPLEX : (bodySettings.motionType == PHBM_DYNAMIC ? EPhysicsLayer::DYNAMIC : EPhysicsLayer::STATIC));

			bodySettings.shapeData = (void*)coll.shape;
			bodySettings.shapeType = coll.shapeType;
			bodySettings.transform = FTransform(GetWorldPosition(), GetWorldRotation(), GetWorldScale());

			auto* body = physWorld->CreateBody(bodySettings);
			if (!body)
				continue;

			if (!physicsBody)
			{
				physicsBody = body;

				if (bWeldToParent)
				{
					auto* p = GetParent();
					while (p)
					{
						if (auto* primitve = Cast<CPrimitiveComponent>(p); primitve)
						{
							//physWorld->CreateConstraint(EConstraint::FIXED, physicsBody, primitive->physicsBody);
							bHasParentBody = true;
							break;
						}

						p = p->GetParent();
					}
				}
				else
				{
					// Detach from the parent.
					Detach();
				}
			}
			else
			{
				physBodies.Add(body);
				//physWorld->CreateConstraint(EConstraint::FIXED, body, physicsBody);
			}
		}

		EnableCollision(CollisionEnabled());
	}
}
