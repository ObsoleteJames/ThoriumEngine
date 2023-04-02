
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/projects/MgsDemo/.project/MGS/intermediate/../../MGS/src/MgsPawn.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_MGS();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CMgsPawn : public FClass
{
public:
	FClass_CMgsPawn()
	{
		name = "Mgs Pawn";
		cppName = "CMgsPawn";
		size = sizeof(CMgsPawn);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CPawn::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_MGS().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CMgsPawn(); }
};
FClass_CMgsPawn __FClass_CMgsPawn_Instance;

FClass* CMgsPawn::StaticClass() { return &__FClass_CMgsPawn_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
