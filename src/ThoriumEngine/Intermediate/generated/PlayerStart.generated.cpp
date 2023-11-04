
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Entities/PlayerStart.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#if IS_DEV
static TPair<FString, FString> _CPlayerStart_tag_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CPlayerStart_tag_Meta {
	"",
	"",
	"",
	"",
	1,
	_CPlayerStart_tag_Meta_Tags
};

#define _CPlayerStart_tag_Meta_Ptr &_CPlayerStart_tag_Meta
#else
#define _CPlayerStart_tag_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CPlayerStart, "Tag", tag, "", "FString", EVT_STRING, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CPlayerStart, tag), sizeof(FString), _CPlayerStart_tag_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CPlayerStart, tag)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CPlayerStart : public FClass
{
public:
	FClass_CPlayerStart()
	{
		name = "Player Start";
		cppName = "CPlayerStart";
		size = sizeof(CPlayerStart);
		numProperties = 1;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CEntity::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CPlayerStart(); }
};
FClass_CPlayerStart __FClass_CPlayerStart_Instance;

FClass* CPlayerStart::StaticClass() { return &__FClass_CPlayerStart_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
