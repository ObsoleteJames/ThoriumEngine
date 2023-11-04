
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Resources/Scene.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#if IS_DEV
static TPair<FString, FString> _CScene_gamemodeClass_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Gamemode" },
};

static FPropertyMeta _CScene_gamemodeClass_Meta {
	"",
	"",
	"Gamemode",
	"",
	2,
	_CScene_gamemodeClass_Meta_Tags
};

#define _CScene_gamemodeClass_Meta_Ptr &_CScene_gamemodeClass_Meta
#else
#define _CScene_gamemodeClass_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CScene, "Gamemode Class", gamemodeClass, "", "CGameMode", EVT_CLASS_PTR, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CScene, gamemodeClass), sizeof(TClassPtr<CGameMode>), _CScene_gamemodeClass_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CScene, gamemodeClass)

#if IS_DEV
static TPair<FString, FString> _CScene_gravity_Meta_Tags[]{
	{ "Editable", "" },
	{ "Category", "Physics" },
};

static FPropertyMeta _CScene_gravity_Meta {
	"",
	"",
	"Physics",
	"",
	2,
	_CScene_gravity_Meta_Tags
};

#define _CScene_gravity_Meta_Ptr &_CScene_gravity_Meta
#else
#define _CScene_gravity_Meta_Ptr nullptr
#endif
DECLARE_PROPERTY(CScene, "Gravity", gravity, "", "float", EVT_FLOAT, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , offsetof(CScene, gravity), sizeof(float), _CScene_gravity_Meta_Ptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CScene, gravity)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FAssetClass_CScene_Tags[] {
	{ "Extension", ".thscene" },
};
#endif

class FAssetClass_CScene : public FAssetClass
{
public:
	FAssetClass_CScene()
	{
		name = "Scene";
		cppName = "CScene";
		size = sizeof(CScene);
		numProperties = 2;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FAssetClass_CScene_Tags;
#endif
		extension = ".thscene";
		BaseClass = CAsset::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		assetFlags = ASSET_NONE;
		GetModule_Engine().RegisterFClass(this);
		GetModule_Engine().RegisterFAsset(this);
	}
	CObject* Instantiate() override { return new CScene(); }
};
FAssetClass_CScene __FAssetClass_CScene_Instance;

FClass* CScene::StaticClass() { return &__FAssetClass_CScene_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
