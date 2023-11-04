
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/EntityComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#if IS_DEV
static TPair<FString, FString> _CEntityComponent_bIsVisible_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Rendering" },
};

static FPropertyMeta _CEntityComponent_bIsVisible_Meta {
	"",
	"",
	"Rendering",
	"",
	2,
	_CEntityComponent_bIsVisible_Meta_Tags
};

#define _CEntityComponent_bIsVisible_Meta_Ptr &_CEntityComponent_bIsVisible_Meta
#else
#define _CEntityComponent_bIsVisible_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntityComponent, "Is Visible", bIsVisible, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntityComponent, bIsVisible), sizeof(bool), _CEntityComponent_bIsVisible_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntityComponent, bIsVisible)

#if IS_DEV
static TPair<FString, FString> _CEntityComponent_bEditorOnly_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CEntityComponent_bEditorOnly_Meta {
	"",
	"",
	"",
	"",
	1,
	_CEntityComponent_bEditorOnly_Meta_Tags
};

#define _CEntityComponent_bEditorOnly_Meta_Ptr &_CEntityComponent_bEditorOnly_Meta
#else
#define _CEntityComponent_bEditorOnly_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CEntityComponent, "Editor Only", bEditorOnly, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CEntityComponent, bEditorOnly), sizeof(bool), _CEntityComponent_bEditorOnly_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntityComponent, bEditorOnly)

DECLARE_PROPERTY(CEntityComponent, "User Created", bUserCreated, "", "bool", EVT_BOOL, VTAG_SERIALIZABLE , CEntityComponent::__private_bUserCreated_offset(), sizeof(bool), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CEntityComponent, bUserCreated)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_CEntityComponent_Tags[] {
	{ "NotCreatable", "" },
};
#endif

class FClass_CEntityComponent : public FClass
{
public:
	FClass_CEntityComponent()
	{
		name = "Entity Component";
		cppName = "CEntityComponent";
		size = sizeof(CEntityComponent);
		numProperties = 3;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_CEntityComponent_Tags;
#endif
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CEntityComponent(); }
};
FClass_CEntityComponent __FClass_CEntityComponent_Instance;

FClass* CEntityComponent::StaticClass() { return &__FClass_CEntityComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
