
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Entities/SunLightEntity.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_CSunLightEntity_Tags[] {
	{ "Name", "Sun Light" },
};
#endif

class FClass_CSunLightEntity : public FClass
{
public:
	FClass_CSunLightEntity()
	{
		name = "Sun Light";
		cppName = "CSunLightEntity";
		size = sizeof(CSunLightEntity);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_CSunLightEntity_Tags;
#endif
		BaseClass = CEntity::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CSunLightEntity(); }
};
FClass_CSunLightEntity __FClass_CSunLightEntity_Instance;

FClass* CSunLightEntity::StaticClass() { return &__FClass_CSunLightEntity_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
