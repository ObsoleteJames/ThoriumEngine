
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Resources/Asset.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FAssetClass_CAsset_Tags[] {
	{ "Abstract", "" },
};
#endif

class FAssetClass_CAsset : public FAssetClass
{
public:
	FAssetClass_CAsset()
	{
		name = "Asset";
		cppName = "CAsset";
		size = sizeof(CAsset);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FAssetClass_CAsset_Tags;
#endif
		BaseClass = CObject::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		assetFlags = ASSET_NONE;
		GetModule_Engine().RegisterFClass(this);
		GetModule_Engine().RegisterFAsset(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FAssetClass_CAsset __FAssetClass_CAsset_Instance;

FClass* CAsset::StaticClass() { return &__FAssetClass_CAsset_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
