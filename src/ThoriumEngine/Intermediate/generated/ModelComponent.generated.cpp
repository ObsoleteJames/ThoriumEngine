
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/ModelComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#if IS_DEV
static TPair<FString, FString> _CModelComponent_model_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Rendering" },
	{ "OnEditFunc", "OnModelEdit" },
};

static FPropertyMeta _CModelComponent_model_Meta {
	"",
	"",
	"Rendering",
	"",
	3,
	_CModelComponent_model_Meta_Tags
};

#define _CModelComponent_model_Meta_Ptr &_CModelComponent_model_Meta
#else
#define _CModelComponent_model_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CModelComponent, "Model", model, "", "CModelAsset", EVT_OBJECT_PTR, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CModelComponent::__private_model_offset(), sizeof(TObjectPtr<CModelAsset>), _CModelComponent_model_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CModelComponent, model)

static FArrayHelper _arrayHelper_materials{
 	[](void* ptr) { (*(TArray<TObjectPtr<CMaterial>>*)ptr).Add(); },
	[](void* ptr, SizeType i) { (*(TArray<TObjectPtr<CMaterial>>*)ptr).Erase((*(TArray<TObjectPtr<CMaterial>>*)ptr).At(i)); },
	[](void* ptr) { (*(TArray<TObjectPtr<CMaterial>>*)ptr).Clear(); },
	[](void* ptr) { return (*(TArray<TObjectPtr<CMaterial>>*)ptr).Size(); }, 
	[](void* ptr) { return (void*)(*(TArray<TObjectPtr<CMaterial>>*)ptr).Data(); }, 
	EVT_OBJECT_PTR, 
	sizeof(TObjectPtr<CMaterial>)
};

#if IS_DEV
static TPair<FString, FString> _CModelComponent_materials_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Rendering" },
};

static FPropertyMeta _CModelComponent_materials_Meta {
	"",
	"",
	"Rendering",
	"",
	2,
	_CModelComponent_materials_Meta_Tags
};

#define _CModelComponent_materials_Meta_Ptr &_CModelComponent_materials_Meta
#else
#define _CModelComponent_materials_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CModelComponent, "Materials", materials, "", "CMaterial", EVT_ARRAY, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CModelComponent::__private_materials_offset(), sizeof(TArray<TObjectPtr<CMaterial>>), _CModelComponent_materials_Meta_Ptr, &_arrayHelper_materials)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CModelComponent, materials)

#if IS_DEV
static TPair<FString, FString> _CModelComponent_bCastShadows_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Rendering" },
};

static FPropertyMeta _CModelComponent_bCastShadows_Meta {
	"",
	"",
	"Rendering",
	"",
	2,
	_CModelComponent_bCastShadows_Meta_Tags
};

#define _CModelComponent_bCastShadows_Meta_Ptr &_CModelComponent_bCastShadows_Meta
#else
#define _CModelComponent_bCastShadows_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CModelComponent, "Cast Shadows", bCastShadows, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CModelComponent::__private_bCastShadows_offset(), sizeof(bool), _CModelComponent_bCastShadows_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CModelComponent, bCastShadows)

static FArrayHelper _arrayHelper_activeBodyGroups{
 	[](void* ptr) { (*(TArray<int>*)ptr).Add(int()); },
	[](void* ptr, SizeType i) { (*(TArray<int>*)ptr).Erase((*(TArray<int>*)ptr).At(i)); },
	[](void* ptr) { (*(TArray<int>*)ptr).Clear(); },
	[](void* ptr) { return (*(TArray<int>*)ptr).Size(); }, 
	[](void* ptr) { return (void*)(*(TArray<int>*)ptr).Data(); }, 
	EVT_INT, 
	sizeof(int)
};

DECLARE_PROPERTY(CModelComponent, "Active Body Groups", activeBodyGroups, "", "int", EVT_ARRAY, VTAG_SERIALIZABLE , CModelComponent::__private_activeBodyGroups_offset(), sizeof(TArray<int>), nullptr, &_arrayHelper_activeBodyGroups)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CModelComponent, activeBodyGroups)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

DECLARE_FUNCTION_PROPERTY(CModelComponent, "OnModelEdit", "", OnModelEdit, &CModelComponent::execOnModelEdit, FFunction::GENERAL, { }, 0)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CModelComponent, OnModelEdit)

class FClass_CModelComponent : public FClass
{
public:
	FClass_CModelComponent()
	{
		name = "Model Component";
		cppName = "CModelComponent";
		size = sizeof(CModelComponent);
		numProperties = 4;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CSceneComponent::StaticClass();
		numFunctions = 1;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CModelComponent(); }
};
FClass_CModelComponent __FClass_CModelComponent_Instance;

FClass* CModelComponent::StaticClass() { return &__FClass_CModelComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION

void CModelComponent::execOnModelEdit(CObject* obj, FStack& stack)
{
	((CModelComponent*)obj)->OnModelEdit();
}
