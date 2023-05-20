
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/GameMode.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#if IS_DEV
static TPair<FString, FString> _CGameMode_playerControllerClass_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CGameMode_playerControllerClass_Meta {
	"",
	"",
	"",
	"",
	1,
	_CGameMode_playerControllerClass_Meta_Tags
};

#define _CGameMode_playerControllerClass_Meta_Ptr &_CGameMode_playerControllerClass_Meta
#else
#define _CGameMode_playerControllerClass_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CGameMode, "Player Controller Class", playerControllerClass, "", "CPlayerController", EVT_CLASS_PTR, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CGameMode::__private_playerControllerClass_offset(), sizeof(TClassPtr<CPlayerController>), _CGameMode_playerControllerClass_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CGameMode, playerControllerClass)

#if IS_DEV
static TPair<FString, FString> _CGameMode_defaultPawnClass_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CGameMode_defaultPawnClass_Meta {
	"",
	"",
	"",
	"",
	1,
	_CGameMode_defaultPawnClass_Meta_Tags
};

#define _CGameMode_defaultPawnClass_Meta_Ptr &_CGameMode_defaultPawnClass_Meta
#else
#define _CGameMode_defaultPawnClass_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CGameMode, "Default Pawn Class", defaultPawnClass, "", "CPawn", EVT_CLASS_PTR, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CGameMode::__private_defaultPawnClass_offset(), sizeof(TClassPtr<CPawn>), _CGameMode_defaultPawnClass_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CGameMode, defaultPawnClass)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CGameMode : public FClass
{
public:
	FClass_CGameMode()
	{
		name = "Game Mode";
		cppName = "CGameMode";
		size = sizeof(CGameMode);
		numProperties = 2;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CGameMode(); }
};
FClass_CGameMode __FClass_CGameMode_Instance;

FClass* CGameMode::StaticClass() { return &__FClass_CGameMode_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
