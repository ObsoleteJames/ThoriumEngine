#pragma once

#include "Asset.h"
#include "DataAsset.generated.h"

/**
 *	an Asset type for generic data storage, uses Object Properties as its method for storing data.
 */
ASSET(Abstract)
class ENGINE_API CDataAsset : public CAsset
{
	GENERATED_BODY()

public:
	CDataAsset() = default;
	
	void OnInit(IBaseFStream* stream) final;

	void OnSave(IBaseFStream* stream) final;
	void OnLoad(IBaseFStream* stream, uint8) final;
	void Unload(uint8) final;
};
