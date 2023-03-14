
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Misc/TransformGizmoEntity.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_EGizmoType : public FEnum
{
	public:
	FEnum_EGizmoType()
	{
		values.Add({ "GIZMO_TRANSLATE", (int64)EGizmoType::GIZMO_TRANSLATE });
		values.Add({ "GIZMO_ROTATE", (int64)EGizmoType::GIZMO_ROTATE });
		values.Add({ "GIZMO_SCALE", (int64)EGizmoType::GIZMO_SCALE });
		name = "GizmoType";
		cppName = "EGizmoType";
		size = sizeof(EGizmoType);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_EGizmoType __FEnum_EGizmoType_Instance;

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CTransformGizmoEntity : public FClass
{
public:
	FClass_CTransformGizmoEntity()
	{
		name = "Transform Gizmo Entity";
		cppName = "CTransformGizmoEntity";
		size = sizeof(CTransformGizmoEntity);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CEntity::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_HIDDEN;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CTransformGizmoEntity(); }
};
FClass_CTransformGizmoEntity __FClass_CTransformGizmoEntity_Instance;

FClass* CTransformGizmoEntity::StaticClass() { return &__FClass_CTransformGizmoEntity_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
