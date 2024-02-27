
#include "BillboardComponent.h"
#include "Resources/Material.h"
#include "Resources/Texture.h"
#include "Game/World.h"

void CBillboardComponent::Init()
{
	class CBillboardPrimitiveProxy : public CPrimitiveProxy
	{
	public:
		CBillboardPrimitiveProxy(CBillboardComponent* obj) : comp(obj)
		{
			owner = obj;
		}

		void FetchData() override
		{
			bVisible = comp->IsVisible();
			if (comp->bEditorOnly && comp->GetWorld()->IsActive())
				bVisible = false;

			if (!bVisible)
				return;

			mat = comp->mat;
			sprite = comp->sprite;

			mat->SetTexture("vBaseColor", sprite);
			mat->Load(0);

			transform.position = comp->GetWorldPosition();
			transform.scale = comp->GetWorldScale();
			matrix = FMatrix(1.f).Translate(transform.position).Scale(transform.scale);
			bounds = comp->Bounds();
		}

		void GetDynamicMeshes(FMeshBuilder& out) override
		{
			FMesh mesh{};
			mesh.numVertices = 6;
			out.DrawMesh(mesh, mat, matrix);
		}

	private:
		CBillboardComponent* comp;
		TObjectPtr<CMaterial> mat;
		TObjectPtr<CTexture> sprite;
	};

	mat = CreateObject<CMaterial>("BillboardMat");
	mat->SetShader("Billboard");
	mat->SetFloat("vAlpha", 1.f);

	bounds.extents = FVector::one * 0.5f;

	sprite = CResourceManager::GetResource<CTexture>("misc/Obsolete.thtex");

	if (GetWorld())
	{
		renderProxy = new CBillboardPrimitiveProxy(this);
		GetWorld()->RegisterPrimitive(renderProxy);
	}
}

void CBillboardComponent::OnDelete()
{
	if (renderProxy)
	{
		GetWorld()->UnregisterPrimitive(renderProxy);
		delete renderProxy;
	}
	mat = nullptr;
}

FBounds CBillboardComponent::Bounds() const
{
	FBounds r = bounds;

	FVector scale = GetWorldScale();
	r.extents = r.extents * scale;
	r.position = r.position * scale + GetWorldPosition();
	return r;
}
