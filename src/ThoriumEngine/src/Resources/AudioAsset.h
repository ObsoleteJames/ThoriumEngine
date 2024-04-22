#pragma once

#include "Asset.h"
#include "AudioAsset.generated.h"

ASSET(Extension = ".wav")
class CAudioAsset : public CAsset
{
	GENERATED_BODY()

public:
	void Init() override;
	void Load(uint8) override;

};
