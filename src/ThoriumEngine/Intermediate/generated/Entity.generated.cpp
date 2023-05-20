
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Entity.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_EEntityType : public FEnum
{
	public:
	FEnum_EEntityType()
	{
		values.Add({ "ENTITY_STATIC", (int64)EEntityType::ENTITY_STATIC });
		values.Add({ "ENTITY_DYNAMIC", (int64)EEntityType::ENTITY_DYNAMIC });
		name = "EntityType";
		cppName = "EEntityType";
		size = sizeof(EEntityType);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_EEntityType __FEnum_EEntityType_Instance;

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(FOutputBinding, "Output Name", outputName, "", "FString", EVT_STRING, VTAG_SERIALIZABLE , offsetof(FOutputBinding, outputName), sizeof(FString), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, outputName)

static FArrayHelper _arrayHelper_arguments{
 	[](void* ptr) { (*(TArray<uint8>*)ptr).Add(uint8()); },
	[](void* ptr, SizeType i) { (*(TArray<uint8>*)ptr).Erase((*(TArray<uint8>*)ptr).At(i)); },
	[](void* ptr) { (*(TArray<uint8>*)ptr).Clear(); },
	[](void* ptr) { return (*(TArray<uint8>*)ptr).Size(); }, 
	[](void* ptr) { return (void*)(*(TArray<uint8>*)ptr).Data(); }, 
	EVT_INT, 
	sizeof(uint8)
};

DECLARE_PROPERTY(FOutputBinding, "Arguments", arguments, "", "uint8", EVT_ARRAY, VTAG_SERIALIZABLE , offsetof(FOutputBinding, arguments), sizeof(TArray<uint8>), nullptr, &_arrayHelper_arguments)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, arguments)

DECLARE_PROPERTY(FOutputBinding, "Target Object", targetObject, "", "CEntity", EVT_OBJECT_PTR, VTAG_SERIALIZABLE , offsetof(FOutputBinding, targetObject), sizeof(TObjectPtr<CEntity>), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, targetObject)

DECLARE_PROPERTY(FOutputBinding, "Target Input", targetInput, "", "FString", EVT_STRING, VTAG_SERIALIZABLE , offsetof(FOutputBinding, targetInput), sizeof(FString), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, targetInput)

class FStruct_FOutputBinding : public FStruct
{
public:
	FStruct_FOutputBinding()
	{
		name = "Output Binding";
		cppName = "FOutputBinding";
		size = sizeof(FOutputBinding);
		numProperties = 4;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FOutputBinding __FStruct_FOutputBinding_Instance;

FStruct* FOutputBinding::StaticStruct() { return &__FStruct_FOutputBinding_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

static TPair<FString, FString> _CEntity_bIsVisible_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Rendering" },
};

static FPropertyMeta _CEntity_bIsVisible_Meta {
	"",
	"",
	"Rendering",
	"",
	2,
	_CEntity_bIsVisible_Meta_Tags
};

DECLARE_PROPERTY(CEntity, "Is Visible", bIsVisible, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bIsVisible), sizeof(bool), &_CEntity_bIsVisible_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bIsVisible)

DECLARE_PROPERTY(CEntity, "Is Enabled", bIsEnabled, "", "bool", EVT_BOOL, VTAG_SERIALIZABLE , offsetof(CEntity, bIsEnabled), sizeof(bool), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bIsEnabled)

static TPair<FString, FString> _CEntity_bOwnerOnlySee_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Rendering" },
};

static FPropertyMeta _CEntity_bOwnerOnlySee_Meta {
	"",
	"",
	"Rendering",
	"",
	2,
	_CEntity_bOwnerOnlySee_Meta_Tags
};

DECLARE_PROPERTY(CEntity, "Owner Only See", bOwnerOnlySee, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bOwnerOnlySee), sizeof(bool), &_CEntity_bOwnerOnlySee_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bOwnerOnlySee)

static TPair<FString, FString> _CEntity_bOwnerCantSee_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Rendering" },
};

static FPropertyMeta _CEntity_bOwnerCantSee_Meta {
	"",
	"",
	"Rendering",
	"",
	2,
	_CEntity_bOwnerCantSee_Meta_Tags
};

DECLARE_PROPERTY(CEntity, "Owner Cant See", bOwnerCantSee, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bOwnerCantSee), sizeof(bool), &_CEntity_bOwnerCantSee_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bOwnerCantSee)

#if INCLUDE_EDITOR_DATA
static TPair<FString, FString> _CEntity_bEditorOnly_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CEntity_bEditorOnly_Meta {
	"",
	"",
	"",
	"",
	1,
	_CEntity_bEditorOnly_Meta_Tags
};

DECLARE_PROPERTY(CEntity, "Editor Only", bEditorOnly, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bEditorOnly), sizeof(bool), &_CEntity_bEditorOnly_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bEditorOnly)
#endif

static TPair<FString, FString> _CEntity_type_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CEntity_type_Meta {
	"",
	"",
	"",
	"",
	1,
	_CEntity_type_Meta_Tags
};

DECLARE_PROPERTY(CEntity, "Type", type, "", "EEntityType", EVT_ENUM, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, type), sizeof(EEntityType), &_CEntity_type_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, type)

static TPair<FString, FString> _CEntity_bCanBeDamaged_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Health" },
};

static FPropertyMeta _CEntity_bCanBeDamaged_Meta {
	"",
	"",
	"Health",
	"",
	2,
	_CEntity_bCanBeDamaged_Meta_Tags
};

DECLARE_PROPERTY(CEntity, "Can Be Damaged", bCanBeDamaged, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bCanBeDamaged), sizeof(bool), &_CEntity_bCanBeDamaged_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bCanBeDamaged)

static TPair<FString, FString> _CEntity_health_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Health" },
};

static FPropertyMeta _CEntity_health_Meta {
	"",
	"",
	"Health",
	"",
	2,
	_CEntity_health_Meta_Tags
};

DECLARE_PROPERTY(CEntity, "Health", health, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, health), sizeof(float), &_CEntity_health_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, health)

static FArrayHelper _arrayHelper_boundOutputs{
 	[](void* ptr) { (*(TArray<FOutputBinding>*)ptr).Add(FOutputBinding()); },
	[](void* ptr, SizeType i) { (*(TArray<FOutputBinding>*)ptr).Erase((*(TArray<FOutputBinding>*)ptr).At(i)); },
	[](void* ptr) { (*(TArray<FOutputBinding>*)ptr).Clear(); },
	[](void* ptr) { return (*(TArray<FOutputBinding>*)ptr).Size(); }, 
	[](void* ptr) { return (void*)(*(TArray<FOutputBinding>*)ptr).Data(); }, 
	EVT_STRUCT, 
	sizeof(FOutputBinding)
};

DECLARE_PROPERTY(CEntity, "Bound Outputs", boundOutputs, "", "FOutputBinding", EVT_ARRAY, VTAG_SERIALIZABLE , CEntity::__private_boundOutputs_offset(), sizeof(TArray<FOutputBinding>), nullptr, &_arrayHelper_boundOutputs)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, boundOutputs)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CEntity : public FClass
{
public:
	FClass_CEntity()
	{
		name = "Entity";
		cppName = "CEntity";
		size = sizeof(CEntity);
		numProperties = 9;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CEntity(); }
};
FClass_CEntity __FClass_CEntity_Instance;

FClass* CEntity::StaticClass() { return &__FClass_CEntity_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
