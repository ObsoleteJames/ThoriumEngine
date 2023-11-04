
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/PlayerController.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_CPlayerController_Tags[] {
	{ "Hidden", "" },
};
#endif

class FClass_CPlayerController : public FClass
{
public:
	FClass_CPlayerController()
	{
		name = "Player Controller";
		cppName = "CPlayerController";
		size = sizeof(CPlayerController);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_CPlayerController_Tags;
#endif
		BaseClass = CPawnController::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_HIDDEN;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CPlayerController(); }
};
FClass_CPlayerController __FClass_CPlayerController_Instance;

FClass* CPlayerController::StaticClass() { return &__FClass_CPlayerController_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
