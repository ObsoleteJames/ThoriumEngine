
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

			position = comp->GetWorldPosition();
			transform = FMatrix(1.f).Translate(position).Scale(comp->GetWorldScale());
		}

		void GetDynamicMeshes(FMeshBuilder& out) override
		{
			FMesh mesh{};
			mesh.numVertices = 6;
			out.DrawMesh(mesh, mat, transform);
		}

	private:
		CBillboardComponent* comp;
		TObjectPtr<CMaterial> mat;
		TObjectPtr<CTexture> sprite;
	};

	mat = CreateObject<CMaterial>();
	mat->SetShader("Billboard");
	mat->SetFloat("vAlpha", 1.f);

	sprite = CResourceManager::GetResource<CTexture>(L"misc\\Obsolete.thtex");

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
		GetWorld()->UnregisterPrimitve(renderProxy);
		delete renderProxy;
	}
}
