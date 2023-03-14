
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/CameraComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_EProjectionType : public FEnum
{
	public:
	FEnum_EProjectionType()
	{
		values.Add({ "PT_PERSPECTIVE", (int64)EProjectionType::PT_PERSPECTIVE });
		values.Add({ "PT_ORTHOGRAPHIC", (int64)EProjectionType::PT_ORTHOGRAPHIC });
		name = "ProjectionType";
		cppName = "EProjectionType";
		size = sizeof(EProjectionType);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_EProjectionType __FEnum_EProjectionType_Instance;

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(CCameraComponent, "Fov", fov, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CCameraComponent::__private_fov_offset(), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCameraComponent, fov)

DECLARE_PROPERTY(CCameraComponent, "Near Plane", nearPlane, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CCameraComponent::__private_nearPlane_offset(), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCameraComponent, nearPlane)

DECLARE_PROPERTY(CCameraComponent, "Far Plane", farPlane, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CCameraComponent::__private_farPlane_offset(), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCameraComponent, farPlane)

DECLARE_PROPERTY(CCameraComponent, "Projection", projection, "", "EProjectionType", EVT_ENUM, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CCameraComponent::__private_projection_offset(), sizeof(EProjectionType), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCameraComponent, projection)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CCameraComponent : public FClass
{
public:
	FClass_CCameraComponent()
	{
		name = "Camera Component";
		cppName = "CCameraComponent";
		size = sizeof(CCameraComponent);
		numProperties = 4;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CSceneComponent::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CCameraComponent(); }
};
FClass_CCameraComponent __FClass_CCameraComponent_Instance;

FClass* CCameraComponent::StaticClass() { return &__FClass_CCameraComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
