
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

static TPair<FString, FString> _CCameraComponent_fov_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CCameraComponent_fov_Meta {
	"",
	"",
	"",
	"",
	1,
	_CCameraComponent_fov_Meta_Tags
};

DECLARE_PROPERTY(CCameraComponent, "Fov", fov, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CCameraComponent, fov), sizeof(float), &_CCameraComponent_fov_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCameraComponent, fov)

static TPair<FString, FString> _CCameraComponent_nearPlane_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CCameraComponent_nearPlane_Meta {
	"",
	"",
	"",
	"",
	1,
	_CCameraComponent_nearPlane_Meta_Tags
};

DECLARE_PROPERTY(CCameraComponent, "Near Plane", nearPlane, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CCameraComponent, nearPlane), sizeof(float), &_CCameraComponent_nearPlane_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCameraComponent, nearPlane)

static TPair<FString, FString> _CCameraComponent_farPlane_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CCameraComponent_farPlane_Meta {
	"",
	"",
	"",
	"",
	1,
	_CCameraComponent_farPlane_Meta_Tags
};

DECLARE_PROPERTY(CCameraComponent, "Far Plane", farPlane, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CCameraComponent, farPlane), sizeof(float), &_CCameraComponent_farPlane_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCameraComponent, farPlane)

static TPair<FString, FString> _CCameraComponent_projection_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CCameraComponent_projection_Meta {
	"",
	"",
	"",
	"",
	1,
	_CCameraComponent_projection_Meta_Tags
};

DECLARE_PROPERTY(CCameraComponent, "Projection", projection, "", "EProjectionType", EVT_ENUM, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CCameraComponent, projection), sizeof(EProjectionType), &_CCameraComponent_projection_Meta, nullptr)
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
