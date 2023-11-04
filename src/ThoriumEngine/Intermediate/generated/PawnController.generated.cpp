
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/PawnController.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_CPawnController_Tags[] {
	{ "Hidden", "" },
};
#endif

class FClass_CPawnController : public FClass
{
public:
	FClass_CPawnController()
	{
		name = "Pawn Controller";
		cppName = "CPawnController";
		size = sizeof(CPawnController);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_CPawnController_Tags;
#endif
		BaseClass = CEntity::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_HIDDEN;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CPawnController(); }
};
FClass_CPawnController __FClass_CPawnController_Instance;

FClass* CPawnController::StaticClass() { return &__FClass_CPawnController_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
