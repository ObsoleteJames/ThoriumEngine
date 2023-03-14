
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/projects/MgsDemo/.project/MGS/intermediate/../../MGS/src/TestEntity.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_MGS();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(FTestStruct, "Tes Float", tesFloat, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FTestStruct, tesFloat), sizeof(float), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FTestStruct, tesFloat)

DECLARE_PROPERTY(FTestStruct, "Tes Int", tesInt, "", "int", EVT_INT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FTestStruct, tesInt), sizeof(int), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FTestStruct, tesInt)

class FStruct_FTestStruct : public FStruct
{
public:
	FStruct_FTestStruct()
	{
		name = "Test Struct";
		cppName = "FTestStruct";
		size = sizeof(FTestStruct);
		numProperties = 2;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = false;
		GetModule_MGS().RegisterFStruct(this);
	}
};
FStruct_FTestStruct __FStruct_FTestStruct_Instance;

FStruct* FTestStruct::StaticStruct() { return &__FStruct_FTestStruct_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(CTestEntity, "Test Int", testInt, "", "int", EVT_INT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CTestEntity, testInt), sizeof(int), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CTestEntity, testInt)

DECLARE_PROPERTY(CTestEntity, "Tes Struct", tesStruct, "", "FTestStruct", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CTestEntity, tesStruct), sizeof(FTestStruct), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CTestEntity, tesStruct)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FClass_CTestEntity : public FClass
{
public:
	FClass_CTestEntity()
	{
		name = "Test Entity";
		cppName = "CTestEntity";
		size = sizeof(CTestEntity);
		numProperties = 2;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		BaseClass = CEntity::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		GetModule_MGS().RegisterFClass(this);
	}
	CObject* Instantiate() override { return new CTestEntity(); }
};
FClass_CTestEntity __FClass_CTestEntity_Instance;

FClass* CTestEntity::StaticClass() { return &__FClass_CTestEntity_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
