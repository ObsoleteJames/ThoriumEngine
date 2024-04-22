
#include "SpriteComponent.h"
#include "Resources/Material.h"
#include "Rendering/RenderScene.h"
#include "Engine.h"

void CSpriteComponent::Init()
{
	BaseClass::Init();
	defaultMat = CreateObject<CMaterial>();
	defaultMat->SetShader("Sprite");

	planeMdl = CResourceManager::GetResource<CModelAsset>("models/Plane.thmdl");

	class SpriteProxy : public CPrimitiveProxy
	{
	public:
		SpriteProxy(CSpriteComponent* c) : comp(c)
		{

		}

		void FetchData() override
		{

		}

	public:
		CSpriteComponent* comp;

	};
}

void CSpriteComponent::SetTexture(CTexture* tex)
{
	if (material)
		return;

	spriteTexture = tex;
}

void CSpriteComponent::SetMaterial(CMaterial* mat)
{
	material = mat;
}

CMaterial* CSpriteComponent::GetFinalMaterial()
{
	if (material)
		return material;
	
	defaultMat->SetTexture("Base Color", spriteTexture);
	return defaultMat;
}

void CSpriteComponent::Render(CRenderScene* context)
{
	if (!spriteTexture && !material)
		return;

	CTexture* s = (material ? material->GetTexture("Base Color") : (CTexture*)spriteTexture);

	FVector2 scale = GetWorldScale();
	scale.x *= (float)s->GetWidth() / 128.f;
	scale.y *= (float)s->GetHeight() / 128.f;

	FMatrix matrix = FMatrix(1.f).Translate(GetWorldPosition()).Rotate2D(GetWorldRotation()).Scale(scale);

	FDrawMeshCmd cmd;
	cmd.drawOrder = zOrder;
	cmd.material = GetFinalMaterial();
	cmd.mesh = gRenderer->GetQuadMesh();
	cmd.drawType = MESH_DRAW_DEFAULT;
	cmd.transform = matrix;

	context->PushCommand(FRenderCommand(cmd, R_FORWARD_PASS));
}
