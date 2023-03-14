
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Resources/ModelAsset.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

class FAssetClass_CModelAsset : public FAssetClass
{
public:
	FAssetClass_CModelAsset()
	{
		name = "Model Asset";
		cppName = "CModelAsset";
		size = sizeof(CModelAsset);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
		extension = ".thmdl";
		BaseClass = CAsset::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		assetFlags = ASSET_NONE;
		GetModule_Engine().RegisterFClass(this);
		GetModule_Engine().RegisterFAsset(this);
	}
	CObject* Instantiate() override { return new CModelAsset(); }
};
FAssetClass_CModelAsset __FAssetClass_CModelAsset_Instance;

FClass* CModelAsset::StaticClass() { return &__FAssetClass_CModelAsset_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
