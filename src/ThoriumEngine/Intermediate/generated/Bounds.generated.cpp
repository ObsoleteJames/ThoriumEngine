
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Math/Bounds.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

class FStruct_FBounds : public FStruct
{
public:
	FStruct_FBounds()
	{
		name = "Bounds";
		cppName = "FBounds";
		size = sizeof(FBounds);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_Engine().RegisterFStruct(this);
	}
};
FStruct_FBounds __FStruct_FBounds_Instance;

FStruct* FBounds::StaticStruct() { return &__FStruct_FBounds_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
