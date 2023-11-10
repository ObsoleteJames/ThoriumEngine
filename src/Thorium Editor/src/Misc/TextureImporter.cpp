
#include "AssetBrowserWidget.h"
#include "Resources/Texture.h"
#include "Console.h"

class FTextureImportAction : public FAssetBrowserAction
{
public:
	FTextureImportAction()
	{
		type = BA_FILE_IMPORT;
		targetClass = (FAssetClass*)CTexture::StaticClass();
	}

	void Invoke(FBrowserActionData* d) override
	{
		FBAImportFile* data = (FBAImportFile*)d;

		CTexture* tex = CreateObject<CTexture>();
		THORIUM_ASSERT(CResourceManager::RegisterNewResource(tex, data->outPath, data->outMod), "Failed to register CTexture asset!");

		if (!tex->Import(ToWString(data->sourceFile)))
			CONSOLE_LogError("CTexture", "Failed to import Texture asset!");
	}
} static FTextureImportAction_instance;
