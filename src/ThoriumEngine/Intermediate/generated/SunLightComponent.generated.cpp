
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Game/Components/SunLightComponent.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(CSunLightComponent, "Intensity", intensity, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CSunLightComponent, intensity), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSunLightComponent, intensity)

DECLARE_PROPERTY(CSunLightComponent, "Color", color, "", "FVector", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CSunLightComponent, color), sizeof(FVector), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSunLightComponent, color)

DECLARE_PROPERTY(CSunLightComponent, "Cast Shadows", bCastShadows, "", "bool", EVT_BOOL, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CSunLightComponent, bCastShadows), sizeof(bool), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CSunLightComponent, bCastShadows)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CSunLightComponent : public FClass
{
public:
	FClass_CSunLightComponent()
	{
		name = "Sun Light Component";
		cppName = "CSunLightComponent";
		size = sizeof(CSunLightComponent);
		numProperties = 3;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CSceneComponent::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CSunLightComponent(); }
};
FClass_CSunLightComponent __FClass_CSunLightComponent_Instance;

FClass* CSunLightComponent::StaticClass() { return &__FClass_CSunLightComponent_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
