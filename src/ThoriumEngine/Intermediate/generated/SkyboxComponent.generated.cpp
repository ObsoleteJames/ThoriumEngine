
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/SkyboxComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

static TPair<FString, FString> _CSkyboxComponent_mat_Meta_Tags[]{
	{ "Editable", "" },
	{ "OnEditFunc", "OnMaterialChanged" },
};

static FPropertyMeta _CSkyboxComponent_mat_Meta {
	"",
	"",
	"",
	"",
	2,
	_CSkyboxComponent_mat_Meta_Tags
};

DECLARE_PROPERTY(CSkyboxComponent, "Mat", mat, "", "CMaterial", EVT_OBJECT_PTR, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CSkyboxComponent, mat), sizeof(TObjectPtr<CMaterial>), &_CSkyboxComponent_mat_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSkyboxComponent, mat)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

DECLARE_FUNCTION_PROPERTY(CSkyboxComponent, "OnMaterialChanged", "", OnMaterialChanged, &CSkyboxComponent::execOnMaterialChanged, FFunction::GENERAL, { }, 0)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CSkyboxComponent, OnMaterialChanged)

class FClass_CSkyboxComponent : public FClass
{
public:
	FClass_CSkyboxComponent()
	{
		name = "Skybox Component";
		cppName = "CSkyboxComponent";
		size = sizeof(CSkyboxComponent);
		numProperties = 1;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CModelComponent::StaticClass();
		numFunctions = 1;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CSkyboxComponent(); }
};
FClass_CSkyboxComponent __FClass_CSkyboxComponent_Instance;

FClass* CSkyboxComponent::StaticClass() { return &__FClass_CSkyboxComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION

void CSkyboxComponent::execOnMaterialChanged(CObject* obj, FStack& stack)
{
	((CSkyboxComponent*)obj)->OnMaterialChanged();
}
