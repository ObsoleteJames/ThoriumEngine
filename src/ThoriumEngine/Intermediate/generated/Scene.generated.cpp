
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Resources/Scene.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

DECLARE_PROPERTY(CScene, "Gamemode Class", gamemodeClass, "", "CGameMode", EVT_CLASS_PTR, VTAG_EDITOR_EDITABLE | VTAG_SERIALIZABLE , CScene::__private_gamemodeClass_offset(), sizeof(TClassPtr<CGameMode>), nullptr, nullptr)
#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY &##EVALUATE_PROPERTY_NAME(CScene, gamemodeClass)

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FAssetClass_CScene : public FAssetClass
{
public:
	FAssetClass_CScene()
	{
		name = "Scene";
		cppName = "CScene";
		size = sizeof(CScene);
		numProperties = 1;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
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
