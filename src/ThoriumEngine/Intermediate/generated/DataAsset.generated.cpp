
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Resources/DataAsset.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FAssetClass_CDataAsset_Tags[] {
	{ "Abstract", "" },
};
#endif

class FAssetClass_CDataAsset : public FAssetClass
{
public:
	FAssetClass_CDataAsset()
	{
		name = "Data Asset";
		cppName = "CDataAsset";
		size = sizeof(CDataAsset);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FAssetClass_CDataAsset_Tags;
#endif
		BaseClass = CAsset::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_ABSTRACT;
		assetFlags = ASSET_NONE;
		GetModule_Engine().RegisterFClass(this);
		GetModule_Engine().RegisterFAsset(this);
	}
	CObject* Instantiate() override { return nullptr; }
};
FAssetClass_CDataAsset __FAssetClass_CDataAsset_Instance;

FClass* CDataAsset::StaticClass() { return &__FAssetClass_CDataAsset_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
