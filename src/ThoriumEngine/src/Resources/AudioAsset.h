#pragma once

#include "Asset.h"
#include "AudioAsset.generated.h"

// TODO: add classes for each audio file type. (e.g. CMp3Asset, CWavAsset)

ASSET(Abstract)
class ENGINE_API CAudioAsset : public CAsset
{
	GENERATED_BODY()

public:
	void Init() override;
	void Load(uint8) override;

};
