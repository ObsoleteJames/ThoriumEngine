
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/projects/MgsDemo/.project/MGS/intermediate/../../MGS/src/MgsGameMode.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_MGS();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CMgsGameMode : public FClass
{
public:
	FClass_CMgsGameMode()
	{
		name = "Mgs Game Mode";
		cppName = "CMgsGameMode";
		size = sizeof(CMgsGameMode);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CGameMode::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_MGS().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CMgsGameMode(); }
};
FClass_CMgsGameMode __FClass_CMgsGameMode_Instance;

FClass* CMgsGameMode::StaticClass() { return &__FClass_CMgsGameMode_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
