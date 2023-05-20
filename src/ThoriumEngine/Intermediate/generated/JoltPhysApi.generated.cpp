
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Physics/Jolt/JoltPhysApi.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CJoltPhysApi : public FClass
{
public:
	FClass_CJoltPhysApi()
	{
		name = "Jolt Phys Api";
		cppName = "CJoltPhysApi";
		size = sizeof(CJoltPhysApi);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = IPhysicsApi::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CJoltPhysApi(); }
};
FClass_CJoltPhysApi __FClass_CJoltPhysApi_Instance;

FClass* CJoltPhysApi::StaticClass() { return &__FClass_CJoltPhysApi_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
