
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/BillboardComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

static TPair<FString, FString> _CBillboardComponent_sprite_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CBillboardComponent_sprite_Meta {
	"",
	"",
	"",
	"",
	1,
	_CBillboardComponent_sprite_Meta_Tags
};

DECLARE_PROPERTY(CBillboardComponent, "Sprite", sprite, "", "CTexture", EVT_OBJECT_PTR, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CBillboardComponent, sprite), sizeof(TObjectPtr<CTexture>), &_CBillboardComponent_sprite_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CBillboardComponent, sprite)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CBillboardComponent : public FClass
{
public:
	FClass_CBillboardComponent()
	{
		name = "Billboard Component";
		cppName = "CBillboardComponent";
		size = sizeof(CBillboardComponent);
		numProperties = 1;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CSceneComponent::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CBillboardComponent(); }
};
FClass_CBillboardComponent __FClass_CBillboardComponent_Instance;

FClass* CBillboardComponent::StaticClass() { return &__FClass_CBillboardComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
