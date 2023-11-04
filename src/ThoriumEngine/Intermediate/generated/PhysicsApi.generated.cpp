
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Physics/PhysicsApi.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_IPhysicsApi_Tags[] {
	{ "Abstract", "" },
};
#endif

class FClass_IPhysicsApi : public FClass
{
public:
	FClass_IPhysicsApi()
	{
		name = "Physics Api";
		cppName = "IPhysicsApi";
		size = sizeof(IPhysicsApi);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_IPhysicsApi_Tags;
#endif
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_IPhysicsApi __FClass_IPhysicsApi_Instance;

FClass* IPhysicsApi::StaticClass() { return &__FClass_IPhysicsApi_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
