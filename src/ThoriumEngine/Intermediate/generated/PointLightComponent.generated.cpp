
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/PointLightComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

static TPair<FString, FString> _CPointLightComponent_intensity_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CPointLightComponent_intensity_Meta {
	"",
	"",
	"",
	"",
	1,
	_CPointLightComponent_intensity_Meta_Tags
};

DECLARE_PROPERTY(CPointLightComponent, "Intensity", intensity, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CPointLightComponent, intensity), sizeof(float), &_CPointLightComponent_intensity_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CPointLightComponent, intensity)

static TPair<FString, FString> _CPointLightComponent_range_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CPointLightComponent_range_Meta {
	"",
	"",
	"",
	"",
	1,
	_CPointLightComponent_range_Meta_Tags
};

DECLARE_PROPERTY(CPointLightComponent, "Range", range, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CPointLightComponent, range), sizeof(float), &_CPointLightComponent_range_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CPointLightComponent, range)

static TPair<FString, FString> _CPointLightComponent_color_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CPointLightComponent_color_Meta {
	"",
	"",
	"",
	"",
	1,
	_CPointLightComponent_color_Meta_Tags
};

DECLARE_PROPERTY(CPointLightComponent, "Color", color, "", "FVector", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CPointLightComponent, color), sizeof(FVector), &_CPointLightComponent_color_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CPointLightComponent, color)

static TPair<FString, FString> _CPointLightComponent_bCastShadows_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CPointLightComponent_bCastShadows_Meta {
	"",
	"",
	"",
	"",
	1,
	_CPointLightComponent_bCastShadows_Meta_Tags
};

DECLARE_PROPERTY(CPointLightComponent, "Cast Shadows", bCastShadows, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CPointLightComponent, bCastShadows), sizeof(bool), &_CPointLightComponent_bCastShadows_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CPointLightComponent, bCastShadows)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CPointLightComponent : public FClass
{
public:
	FClass_CPointLightComponent()
	{
		name = "Point Light Component";
		cppName = "CPointLightComponent";
		size = sizeof(CPointLightComponent);
		numProperties = 4;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CSceneComponent::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CPointLightComponent(); }
};
FClass_CPointLightComponent __FClass_CPointLightComponent_Instance;

FClass* CPointLightComponent::StaticClass() { return &__FClass_CPointLightComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
