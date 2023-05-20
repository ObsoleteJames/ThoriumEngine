
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/UserInterface/Canvas.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_ECanvasScaleMode : public FEnum
{
	public:
	FEnum_ECanvasScaleMode()
	{
		values.Add({ "CSM_ScaleToScreen", (int64)ECanvasScaleMode::CSM_ScaleToScreen });
		values.Add({ "CSM_ConstantSize", (int64)ECanvasScaleMode::CSM_ConstantSize });
		name = "CanvasScaleMode";
		cppName = "ECanvasScaleMode";
		size = sizeof(ECanvasScaleMode);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_ECanvasScaleMode __FEnum_ECanvasScaleMode_Instance;

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

static TPair<FString, FString> _CCanvas_bEnabled_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CCanvas_bEnabled_Meta {
	"",
	"",
	"",
	"",
	1,
	_CCanvas_bEnabled_Meta_Tags
};

DECLARE_PROPERTY(CCanvas, "Enabled", bEnabled, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CCanvas, bEnabled), sizeof(bool), &_CCanvas_bEnabled_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCanvas, bEnabled)

static TPair<FString, FString> _CCanvas_scaleMode_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CCanvas_scaleMode_Meta {
	"",
	"",
	"",
	"",
	1,
	_CCanvas_scaleMode_Meta_Tags
};

DECLARE_PROPERTY(CCanvas, "Scale Mode", scaleMode, "", "ECanvasScaleMode", EVT_ENUM, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CCanvas, scaleMode), sizeof(ECanvasScaleMode), &_CCanvas_scaleMode_Meta, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CCanvas, scaleMode)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CCanvas : public FClass
{
public:
	FClass_CCanvas()
	{
		name = "Canvas";
		cppName = "CCanvas";
		size = sizeof(CCanvas);
		numProperties = 2;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CCanvas(); }
};
FClass_CCanvas __FClass_CCanvas_Instance;

FClass* CCanvas::StaticClass() { return &__FClass_CCanvas_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
