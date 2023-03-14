
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Object/Object.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

static FPropertyMeta _CObject_bReplicated_Meta {
	"",
	"",
	"Networking",
	"",
	nullptr
};

DECLARE_PROPERTY(CObject, "Replicated", bReplicated, "\n   Should this object be recplicated to other clients\n  ", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CObject, bReplicated), sizeof(bool), &_CObject_bReplicated_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CObject, bReplicated)

static FPropertyMeta _CObject_NetPriority_Meta {
	"",
	"",
	"Networking",
	"",
	nullptr
};

DECLARE_PROPERTY(CObject, "Net Priority", NetPriority, "\n   Determines whether this object has net priority over others.\n  ", "uint8", EVT_INT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CObject, NetPriority), sizeof(uint8), &_CObject_NetPriority_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CObject, NetPriority)

DECLARE_PROPERTY(CObject, "Name", name, "", "FString", EVT_STRING, VTAG_SERIALIZABLE , CObject::__private_name_offset(), sizeof(FString), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CObject, name)

DECLARE_PROPERTY(CObject, "Owner", Owner, "", "CObject", EVT_OBJECT_PTR, VTAG_SERIALIZABLE , CObject::__private_Owner_offset(), sizeof(TObjectPtr<CObject>), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CObject, Owner)

static FArrayHelper _arrayHelper_Children{
 	[](void* ptr) { (*(TArray<TObjectPtr<CObject>>*)ptr).Add(); },
	[](void* ptr, SizeType i) { (*(TArray<TObjectPtr<CObject>>*)ptr).Erase((*(TArray<TObjectPtr<CObject>>*)ptr).At(i)); },
	[](void* ptr) { (*(TArray<TObjectPtr<CObject>>*)ptr).Clear(); },
	[](void* ptr) { return (*(TArray<TObjectPtr<CObject>>*)ptr).Size(); }, 
	[](void* ptr) { return (void*)(*(TArray<TObjectPtr<CObject>>*)ptr).Data(); }, 
	EVT_OBJECT_PTR, 
	sizeof(TObjectPtr<CObject>)
};

DECLARE_PROPERTY(CObject, "Children", Children, "", "CObject", EVT_ARRAY, VTAG_SERIALIZABLE , CObject::__private_Children_offset(), sizeof(TArray<TObjectPtr<CObject>>), nullptr, &_arrayHelper_Children)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CObject, Children)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

DECLARE_FUNCTION_PROPERTY(CObject, "OnNetDelete", "", OnNetDelete, &CObject::execOnNetDelete, FFunction::MULTICAST_RPC, { }, 0)
#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION &EVALUATE_FUNCTION_NAME(CObject, OnNetDelete)

class FClass_CObject : public FClass
{
public:
	FClass_CObject()
	{
		name = "Object";
		cppName = "CObject";
		size = sizeof(CObject);
		numProperties = 5;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = nullptr;
		numFunctions = 1;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_CObject __FClass_CObject_Instance;

FClass* CObject::StaticClass() { return &__FClass_CObject_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION

void CObject::execOnNetDelete(CObject* obj, FStack& stack)
{
	((CObject*)obj)->OnNetDelete_Implementation();
}
