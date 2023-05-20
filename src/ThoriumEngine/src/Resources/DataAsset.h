#pragma once

#include "Asset.h"
#include "DataAsset.generated.h"

/**
 *	an Asset for generic data storage, uses Object Properties as its method for storing data.
 */
ASSET(Abstract)
class ENGINE_API CDataAsset : public CAsset
{
	GENERATED_BODY()

public:
	CDataAsset() = default;
	
	void Init() final;

	void Save() final;
	void Load(uint8) final;
	void Unload(uint8) final;

	bool Import(const WString&) final;

};
