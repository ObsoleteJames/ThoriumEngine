
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/SceneComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(CSceneComponent, "Parent", parent, "", "CSceneComponent", EVT_OBJECT_PTR, VTAG_SERIALIZABLE , CSceneComponent::__private_parent_offset(), sizeof(TObjectPtr<CSceneComponent>), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSceneComponent, parent)

static FArrayHelper _arrayHelper_children{
 	[](void* ptr) { (*(TArray<TObjectPtr<CSceneComponent>>*)ptr).Add(); },
	[](void* ptr, SizeType i) { (*(TArray<TObjectPtr<CSceneComponent>>*)ptr).Erase((*(TArray<TObjectPtr<CSceneComponent>>*)ptr).At(i)); },
	[](void* ptr) { (*(TArray<TObjectPtr<CSceneComponent>>*)ptr).Clear(); },
	[](void* ptr) { return (*(TArray<TObjectPtr<CSceneComponent>>*)ptr).Size(); }, 
	[](void* ptr) { return (void*)(*(TArray<TObjectPtr<CSceneComponent>>*)ptr).Data(); }, 
	EVT_OBJECT_PTR, 
	sizeof(TObjectPtr<CSceneComponent>)
};

DECLARE_PROPERTY(CSceneComponent, "Children", children, "", "CSceneComponent", EVT_ARRAY, VTAG_SERIALIZABLE , CSceneComponent::__private_children_offset(), sizeof(TArray<TObjectPtr<CSceneComponent>>), nullptr, &_arrayHelper_children)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSceneComponent, children)

DECLARE_PROPERTY(CSceneComponent, "Position", position, "", "FVector", EVT_STRUCT, VTAG_SERIALIZABLE , CSceneComponent::__private_position_offset(), sizeof(FVector), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSceneComponent, position)

DECLARE_PROPERTY(CSceneComponent, "Scale", scale, "", "FVector", EVT_STRUCT, VTAG_SERIALIZABLE , CSceneComponent::__private_scale_offset(), sizeof(FVector), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSceneComponent, scale)

DECLARE_PROPERTY(CSceneComponent, "Rotation", rotation, "", "FQuaternion", EVT_STRUCT, VTAG_SERIALIZABLE , CSceneComponent::__private_rotation_offset(), sizeof(FQuaternion), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSceneComponent, rotation)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CSceneComponent : public FClass
{
public:
	FClass_CSceneComponent()
	{
		name = "Scene Component";
		cppName = "CSceneComponent";
		size = sizeof(CSceneComponent);
		numProperties = 5;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CEntityComponent::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CSceneComponent(); }
};
FClass_CSceneComponent __FClass_CSceneComponent_Instance;

FClass* CSceneComponent::StaticClass() { return &__FClass_CSceneComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
