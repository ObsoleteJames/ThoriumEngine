
#include <Util/Core.h>
#include "F:/MyProjects/VS_Projects/ThoriumEngine/src/ThoriumEngine/src/Rendering/Shader.h"
#include "Object/Class.h"
#include "Module.h"

CModule& GetModule_Engine();

#undef CLASS_NEXT_PROPERTY
#define CLASS_NEXT_PROPERTY nullptr

#undef CLASS_NEXT_FUNCTION
#define CLASS_NEXT_FUNCTION nullptr

#ifdef IS_DEV
static TPair<FString, FString> _FAssetClass_CShaderSource_Tags[] {
	{ "Extension", ".thcs" },
	{ "Hidden", "" },
	{ "AutoLoad", "" },
};
#endif

class FAssetClass_CShaderSource : public FAssetClass
{
public:
	FAssetClass_CShaderSource()
	{
		name = "Shader Source";
		cppName = "CShaderSource";
		size = sizeof(CShaderSource);
		numProperties = 0;
		PropertyList = CLASS_NEXT_PROPERTY;
		bIsClass = true;
#ifdef IS_DEV
		numTags = 3;
		tags = _FAssetClass_CShaderSource_Tags;
#endif
		extension = ".thcs";
		BaseClass = CAsset::StaticClass();
		numFunctions = 0;
		FunctionList = CLASS_NEXT_FUNCTION;
		flags = CTAG_HIDDEN;
		assetFlags = ASSET_NONE | ASSET_AUTO_LOAD;
		GetModule_Engine().RegisterFClass(this);
		GetModule_Engine().RegisterFAsset(this);
	}
	CObject* Instantiate() override { return new CShaderSource(); }
};
FAssetClass_CShaderSource __FAssetClass_CShaderSource_Instance;

FClass* CShaderSource::StaticClass() { return &__FAssetClass_CShaderSource_Instance; }

#undef CLASS_NEXT_PROPERTY
#undef CLASS_NEXT_FUNCTION
