
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

DECLARE_PROPERTY(FOutputBinding, "Target Object", targetObject, "", "FObjectHandle", EVT_STRUCT, VTAG_SERIALIZABLE , offsetof(FOutputBinding, targetObject), sizeof(FObjectHandle), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, targetObject)

DECLARE_PROPERTY(FOutputBinding, "Target Input", targetInput, "", "FString", EVT_STRING, VTAG_SERIALIZABLE , offsetof(FOutputBinding, targetInput), sizeof(FString), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, targetInput)

DECLARE_PROPERTY(FOutputBinding, "Delay", delay, "", "float", EVT_FLOAT, VTAG_SERIALIZABLE , offsetof(FOutputBinding, delay), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, delay)

DECLARE_PROPERTY(FOutputBinding, "Only Once", bOnlyOnce, "", "bool", EVT_BOOL, VTAG_SERIALIZABLE , offsetof(FOutputBinding, bOnlyOnce), sizeof(bool), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, bOnlyOnce)

DECLARE_PROPERTY(FOutputBinding, "Fire Count", fireCount, "", "int", EVT_INT, VTAG_SERIALIZABLE , FOutputBinding::__private_fireCount_offset(), sizeof(int), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FOutputBinding, fireCount)

class FStruct_FOutputBinding : public FStruct
{
public:
	FStruct_FOutputBinding()
	{
		name = "Output Binding";
		cppName = "FOutputBinding";
		size = sizeof(FOutputBinding);
		numProperties = 7;
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

#if IS_DEV
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

#define _CEntity_bIsVisible_Meta_Ptr &_CEntity_bIsVisible_Meta
#else
#define _CEntity_bIsVisible_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntity, "Is Visible", bIsVisible, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bIsVisible), sizeof(bool), _CEntity_bIsVisible_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bIsVisible)

DECLARE_PROPERTY(CEntity, "Is Enabled", bIsEnabled, "", "bool", EVT_BOOL, VTAG_SERIALIZABLE , offsetof(CEntity, bIsEnabled), sizeof(bool), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bIsEnabled)

#if IS_DEV
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

#define _CEntity_bOwnerOnlySee_Meta_Ptr &_CEntity_bOwnerOnlySee_Meta
#else
#define _CEntity_bOwnerOnlySee_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntity, "Owner Only See", bOwnerOnlySee, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bOwnerOnlySee), sizeof(bool), _CEntity_bOwnerOnlySee_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bOwnerOnlySee)

#if IS_DEV
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

#define _CEntity_bOwnerCantSee_Meta_Ptr &_CEntity_bOwnerCantSee_Meta
#else
#define _CEntity_bOwnerCantSee_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntity, "Owner Cant See", bOwnerCantSee, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bOwnerCantSee), sizeof(bool), _CEntity_bOwnerCantSee_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bOwnerCantSee)

#if INCLUDE_EDITOR_DATA
#if IS_DEV
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

#define _CEntity_bEditorOnly_Meta_Ptr &_CEntity_bEditorOnly_Meta
#else
#define _CEntity_bEditorOnly_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntity, "Editor Only", bEditorOnly, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bEditorOnly), sizeof(bool), _CEntity_bEditorOnly_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bEditorOnly)
#endif

#if IS_DEV
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

#define _CEntity_type_Meta_Ptr &_CEntity_type_Meta
#else
#define _CEntity_type_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntity, "Type", type, "", "EEntityType", EVT_ENUM, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, type), sizeof(EEntityType), _CEntity_type_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, type)

#if IS_DEV
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

#define _CEntity_bCanBeDamaged_Meta_Ptr &_CEntity_bCanBeDamaged_Meta
#else
#define _CEntity_bCanBeDamaged_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntity, "Can Be Damaged", bCanBeDamaged, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, bCanBeDamaged), sizeof(bool), _CEntity_bCanBeDamaged_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntity, bCanBeDamaged)

#if IS_DEV
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

#define _CEntity_health_Meta_Ptr &_CEntity_health_Meta
#else
#define _CEntity_health_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntity, "Health", health, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntity, health), sizeof(float), _CEntity_health_Meta_Ptr, nullptr)
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

static FFuncArg _funcArgs_CEntity_SetWorldPosition[] = {
	{ "p", EVT_STRUCT, VTAG_NONE, FVector::StaticStruct() },
};

DECLARE_FUNCTION_PROPERTY(CEntity, "SetWorldPosition", "", SetWorldPosition, &CEntity::execSetWorldPosition, FFunction::GENERAL, _funcArgs_CEntity_SetWorldPosition, 1, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, SetWorldPosition)

static FFuncArg _funcArgs_CEntity_SetPosition[] = {
	{ "p", EVT_STRUCT, VTAG_NONE, FVector::StaticStruct() },
};

DECLARE_FUNCTION_PROPERTY(CEntity, "SetPosition", "", SetPosition, &CEntity::execSetPosition, FFunction::GENERAL, _funcArgs_CEntity_SetPosition, 1, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, SetPosition)

static FFuncArg _funcArgs_CEntity_SetWorldRotation[] = {
	{ "r", EVT_STRUCT, VTAG_NONE, FQuaternion::StaticStruct() },
};

DECLARE_FUNCTION_PROPERTY(CEntity, "SetWorldRotation", "", SetWorldRotation, &CEntity::execSetWorldRotation, FFunction::GENERAL, _funcArgs_CEntity_SetWorldRotation, 1, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, SetWorldRotation)

static FFuncArg _funcArgs_CEntity_SetRotation[] = {
	{ "r", EVT_STRUCT, VTAG_NONE, FQuaternion::StaticStruct() },
};

DECLARE_FUNCTION_PROPERTY(CEntity, "SetRotation", "", SetRotation, &CEntity::execSetRotation, FFunction::GENERAL, _funcArgs_CEntity_SetRotation, 1, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, SetRotation)

static FFuncArg _funcArgs_CEntity_SetWorldScale[] = {
	{ "s", EVT_STRUCT, VTAG_NONE, FVector::StaticStruct() },
};

DECLARE_FUNCTION_PROPERTY(CEntity, "SetWorldScale", "", SetWorldScale, &CEntity::execSetWorldScale, FFunction::GENERAL, _funcArgs_CEntity_SetWorldScale, 1, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, SetWorldScale)

static FFuncArg _funcArgs_CEntity_SetScale[] = {
	{ "s", EVT_STRUCT, VTAG_NONE, FVector::StaticStruct() },
};

DECLARE_FUNCTION_PROPERTY(CEntity, "SetScale", "", SetScale, &CEntity::execSetScale, FFunction::GENERAL, _funcArgs_CEntity_SetScale, 1, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, SetScale)

DECLARE_FUNCTION_PROPERTY(CEntity, "OnStart", "", outputOnStart, &CEntity::execoutputOnStart, FFunction::OUTPUT, nullptr, 0, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, outputOnStart)

DECLARE_FUNCTION_PROPERTY(CEntity, "Hide", "", Hide, &CEntity::execHide, FFunction::GENERAL, nullptr, 0, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, Hide)

DECLARE_FUNCTION_PROPERTY(CEntity, "Show", "", Show, &CEntity::execShow, FFunction::GENERAL, nullptr, 0, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, Show)

DECLARE_FUNCTION_PROPERTY(CEntity, "Enable", "", inputEnable, &CEntity::execinputEnable, FFunction::GENERAL, nullptr, 0, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, inputEnable)

DECLARE_FUNCTION_PROPERTY(CEntity, "Disable", "", inputDisable, &CEntity::execinputDisable, FFunction::GENERAL, nullptr, 0, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, inputDisable)

static FFuncArg _funcArgs_CEntity_inputSetHealth[] = {
	{ "health", EVT_FLOAT, VTAG_NONE, nullptr },
};

DECLARE_FUNCTION_PROPERTY(CEntity, "SetHealth", "", inputSetHealth, &CEntity::execinputSetHealth, FFunction::GENERAL, _funcArgs_CEntity_inputSetHealth, 1, 0, FunctionFlags_NONE | FunctionFlags_ALLOW_AS_INPUT)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CEntity, inputSetHealth)

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
		numFunctions = 12;
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

void CEntity::execSetWorldPosition(CObject* obj, FStack& stack)
{
	POP_STACK_VARIABLE(FVector, p);
	((CEntity*)obj)->SetWorldPosition(p);
}

void CEntity::execSetPosition(CObject* obj, FStack& stack)
{
	POP_STACK_VARIABLE(FVector, p);
	((CEntity*)obj)->SetPosition(p);
}

void CEntity::execSetWorldRotation(CObject* obj, FStack& stack)
{
	POP_STACK_VARIABLE(FQuaternion, r);
	((CEntity*)obj)->SetWorldRotation(r);
}

void CEntity::execSetRotation(CObject* obj, FStack& stack)
{
	POP_STACK_VARIABLE(FQuaternion, r);
	((CEntity*)obj)->SetRotation(r);
}

void CEntity::execSetWorldScale(CObject* obj, FStack& stack)
{
	POP_STACK_VARIABLE(FVector, s);
	((CEntity*)obj)->SetWorldScale(s);
}

void CEntity::execSetScale(CObject* obj, FStack& stack)
{
	POP_STACK_VARIABLE(FVector, s);
	((CEntity*)obj)->SetScale(s);
}

void CEntity::outputOnStart()
{
	FireOutput("OnStart");
}

void CEntity::execoutputOnStart(CObject* obj, FStack& stack)
{
	((CEntity*)obj)->outputOnStart();
}

void CEntity::execHide(CObject* obj, FStack& stack)
{
	((CEntity*)obj)->Hide();
}

void CEntity::execShow(CObject* obj, FStack& stack)
{
	((CEntity*)obj)->Show();
}

void CEntity::execinputEnable(CObject* obj, FStack& stack)
{
	((CEntity*)obj)->inputEnable();
}

void CEntity::execinputDisable(CObject* obj, FStack& stack)
{
	((CEntity*)obj)->inputDisable();
}

void CEntity::execinputSetHealth(CObject* obj, FStack& stack)
{
	POP_STACK_VARIABLE(float, health);
	((CEntity*)obj)->inputSetHealth(health);
}
