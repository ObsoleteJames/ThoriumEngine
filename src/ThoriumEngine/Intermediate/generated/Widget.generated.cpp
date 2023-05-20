
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/UserInterface/Widget.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

static TPair<FString, FString> _CWidget_zOrder_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CWidget_zOrder_Meta {
	"",
	"",
	"",
	"",
	1,
	_CWidget_zOrder_Meta_Tags
};

DECLARE_PROPERTY(CWidget, "ZOrder", zOrder, "", "int", EVT_INT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CWidget, zOrder), sizeof(int), &_CWidget_zOrder_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CWidget, zOrder)

static TPair<FString, FString> _CWidget_position_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CWidget_position_Meta {
	"",
	"",
	"",
	"",
	1,
	_CWidget_position_Meta_Tags
};

DECLARE_PROPERTY(CWidget, "Position", position, "", "FVector2", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CWidget, position), sizeof(FVector2), &_CWidget_position_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CWidget, position)

static TPair<FString, FString> _CWidget_scale_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CWidget_scale_Meta {
	"",
	"",
	"",
	"",
	1,
	_CWidget_scale_Meta_Tags
};

DECLARE_PROPERTY(CWidget, "Scale", scale, "", "FVector2", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CWidget, scale), sizeof(FVector2), &_CWidget_scale_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CWidget, scale)

static TPair<FString, FString> _CWidget_anchorMin_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CWidget_anchorMin_Meta {
	"",
	"",
	"",
	"",
	1,
	_CWidget_anchorMin_Meta_Tags
};

DECLARE_PROPERTY(CWidget, "Anchor Min", anchorMin, "", "FVector2", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CWidget, anchorMin), sizeof(FVector2), &_CWidget_anchorMin_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CWidget, anchorMin)

static TPair<FString, FString> _CWidget_anchorMax_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CWidget_anchorMax_Meta {
	"",
	"",
	"",
	"",
	1,
	_CWidget_anchorMax_Meta_Tags
};

DECLARE_PROPERTY(CWidget, "Anchor Max", anchorMax, "", "FVector2", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CWidget, anchorMax), sizeof(FVector2), &_CWidget_anchorMax_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CWidget, anchorMax)

static TPair<FString, FString> _CWidget_pivot_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CWidget_pivot_Meta {
	"",
	"",
	"",
	"",
	1,
	_CWidget_pivot_Meta_Tags
};

DECLARE_PROPERTY(CWidget, "Pivot", pivot, "", "FVector2", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CWidget, pivot), sizeof(FVector2), &_CWidget_pivot_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CWidget, pivot)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CWidget : public FClass
{
public:
	FClass_CWidget()
	{
		name = "Widget";
		cppName = "CWidget";
		size = sizeof(CWidget);
		numProperties = 6;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CWidget(); }
};
FClass_CWidget __FClass_CWidget_Instance;

FClass* CWidget::StaticClass() { return &__FClass_CWidget_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
