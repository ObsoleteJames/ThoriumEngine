
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Physics/Jolt/JoltPhysWorld.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CJoltPhysWorld : public FClass
{
public:
	FClass_CJoltPhysWorld()
	{
		name = "Jolt Phys World";
		cppName = "CJoltPhysWorld";
		size = sizeof(CJoltPhysWorld);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = IPhysicsWorld::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_CJoltPhysWorld __FClass_CJoltPhysWorld_Instance;

FClass* CJoltPhysWorld::StaticClass() { return &__FClass_CJoltPhysWorld_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
