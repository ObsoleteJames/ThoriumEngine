
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/LightComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_ILightComponent_Tags[] {
	{ "Abstract", "" },
};
#endif

class FClass_ILightComponent : public FClass
{
public:
	FClass_ILightComponent()
	{
		name = "Light Component";
		cppName = "ILightComponent";
		size = sizeof(ILightComponent);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_ILightComponent_Tags;
#endif
		BaseClass = CSceneComponent::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_ILightComponent __FClass_ILightComponent_Instance;

FClass* ILightComponent::StaticClass() { return &__FClass_ILightComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
