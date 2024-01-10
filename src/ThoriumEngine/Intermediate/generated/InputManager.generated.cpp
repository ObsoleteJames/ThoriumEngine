
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEditor/../ThoriumEngine/src/Game/Input/InputManager.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

class FEnum_EInputMode : public FEnum
{
	public:
	FEnum_EInputMode()
	{
		values.Add({ "GAME_ONLY", (int64)EInputMode::GAME_ONLY });
		values.Add({ "UI_ONLY", (int64)EInputMode::UI_ONLY });
		values.Add({ "GAME_UI", (int64)EInputMode::GAME_UI });
		name = "InputMode";
		cppName = "EInputMode";
		size = sizeof(EInputMode);
		flags = EnumFlag_NONE;
		GetModule_Engine().RegisterFEnum(this);
	}
};
FEnum_EInputMode __FEnum_EInputMode_Instance;

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CInputManager : public FClass
{
public:
	FClass_CInputManager()
	{
		name = "Input Manager";
		cppName = "CInputManager";
		size = sizeof(CInputManager);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_Engine().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CInputManager(); }
};
FClass_CInputManager __FClass_CInputManager_Instance;

FClass* CInputManager::StaticClass() { return &__FClass_CInputManager_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
