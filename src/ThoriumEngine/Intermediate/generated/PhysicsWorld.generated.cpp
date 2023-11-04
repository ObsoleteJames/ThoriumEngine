
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Physics/PhysicsWorld.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_EPhysicsLayer : public FEnum
{
	public:
	FEnum_EPhysicsLayer()
	{
		values.Add({ "STATIC", (int64)EPhysicsLayer::STATIC });
		values.Add({ "DYNAMIC", (int64)EPhysicsLayer::DYNAMIC });
		values.Add({ "COMPLEX", (int64)EPhysicsLayer::COMPLEX });
		values.Add({ "TRIGGER", (int64)EPhysicsLayer::TRIGGER });
		values.Add({ "CUSTOM_0", (int64)EPhysicsLayer::CUSTOM_0 });
		values.Add({ "CUSTOM_1", (int64)EPhysicsLayer::CUSTOM_1 });
		values.Add({ "CUSTOM_2", (int64)EPhysicsLayer::CUSTOM_2 });
		values.Add({ "CUSTOM_3", (int64)EPhysicsLayer::CUSTOM_3 });
		values.Add({ "CUSTOM_4", (int64)EPhysicsLayer::CUSTOM_4 });
		values.Add({ "CUSTOM_5", (int64)EPhysicsLayer::CUSTOM_5 });
		values.Add({ "END", (int64)EPhysicsLayer::END });
		name = "PhysicsLayer";
		cppName = "EPhysicsLayer";
		size = sizeof(EPhysicsLayer);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_EPhysicsLayer __FEnum_EPhysicsLayer_Instance;

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FClass_IPhysicsWorld_Tags[] {
	{ "Abstract", "" },
};
#endif

class FClass_IPhysicsWorld : public FClass
{
public:
	FClass_IPhysicsWorld()
	{
		name = "Physics World";
		cppName = "IPhysicsWorld";
		size = sizeof(IPhysicsWorld);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FClass_IPhysicsWorld_Tags;
#endif
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FClass_IPhysicsWorld __FClass_IPhysicsWorld_Instance;

FClass* IPhysicsWorld::StaticClass() { return &__FClass_IPhysicsWorld_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
