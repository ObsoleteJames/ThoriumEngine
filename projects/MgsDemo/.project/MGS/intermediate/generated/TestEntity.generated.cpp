
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/projects/MgsDemo/.project/MGS/src/TestEntity.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_MGS();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#if IS_DEV
static TPair<FString, FString> _FTestStruct_tesFloat_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _FTestStruct_tesFloat_Meta {
	"",
	"",
	"",
	"",
	1,
	_FTestStruct_tesFloat_Meta_Tags
};

#define _FTestStruct_tesFloat_Meta_Ptr &_FTestStruct_tesFloat_Meta
#else
#define _FTestStruct_tesFloat_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(FTestStruct, "Tes Float", tesFloat, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FTestStruct, tesFloat), sizeof(float), _FTestStruct_tesFloat_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(FTestStruct, tesFloat)

#if IS_DEV
static TPair<FString, FString> _FTestStruct_tesInt_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _FTestStruct_tesInt_Meta {
	"",
	"",
	"",
	"",
	1,
	_FTestStruct_tesInt_Meta_Tags
};

#define _FTestStruct_tesInt_Meta_Ptr &_FTestStruct_tesInt_Meta
#else
#define _FTestStruct_tesInt_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(FTestStruct, "Tes Int", tesInt, "", "int", EVT_INT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(FTestStruct, tesInt), sizeof(int), _FTestStruct_tesInt_Meta_Ptr, nullptr)
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

#if IS_DEV
static TPair<FString, FString> _CTestEntity_testInt_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CTestEntity_testInt_Meta {
	"",
	"",
	"",
	"",
	1,
	_CTestEntity_testInt_Meta_Tags
};

#define _CTestEntity_testInt_Meta_Ptr &_CTestEntity_testInt_Meta
#else
#define _CTestEntity_testInt_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CTestEntity, "Test Int", testInt, "", "int", EVT_INT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CTestEntity, testInt), sizeof(int), _CTestEntity_testInt_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CTestEntity, testInt)

#if IS_DEV
static TPair<FString, FString> _CTestEntity_tesStruct_Meta_Tags[]{
	{ "Editable", "" },
};

static FPropertyMeta _CTestEntity_tesStruct_Meta {
	"",
	"",
	"",
	"",
	1,
	_CTestEntity_tesStruct_Meta_Tags
};

#define _CTestEntity_tesStruct_Meta_Ptr &_CTestEntity_tesStruct_Meta
#else
#define _CTestEntity_tesStruct_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CTestEntity, "Tes Struct", tesStruct, "", "FTestStruct", EVT_STRUCT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CTestEntity, tesStruct), sizeof(FTestStruct), _CTestEntity_tesStruct_Meta_Ptr, nullptr)
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
