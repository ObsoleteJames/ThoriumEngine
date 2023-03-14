
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Entities/PointLightEntity.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CPointLightEntity : public FClass
{
public:
	FClass_CPointLightEntity()
	{
		name = "Point Light";
		cppName = "CPointLightEntity";
		size = sizeof(CPointLightEntity);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CEntity::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CPointLightEntity(); }
};
FClass_CPointLightEntity __FClass_CPointLightEntity_Instance;

FClass* CPointLightEntity::StaticClass() { return &__FClass_CPointLightEntity_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
