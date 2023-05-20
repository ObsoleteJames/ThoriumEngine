
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Resources/Material.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FAssetClass_CMaterial_Tags[] {
	{ "Extension", ".thmat" },
};
#endif

class FAssetClass_CMaterial : public FAssetClass
{
public:
	FAssetClass_CMaterial()
	{
		name = "Material";
		cppName = "CMaterial";
		size = sizeof(CMaterial);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 1;
		tags = _FAssetClass_CMaterial_Tags;
#endif
		extension = ".thmat";
		BaseClass = CAsset::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_NONE;
		assetFlags = ASSET_NONE;
		GetModule_Engine().RegisterFClass(this);
		GetModule_Engine().RegisterFAsset(this);
	}
	CObject* Instantiate() override { return new CMaterial(); }
};
FAssetClass_CMaterial __FAssetClass_CMaterial_Instance;

FClass* CMaterial::StaticClass() { return &__FAssetClass_CMaterial_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
