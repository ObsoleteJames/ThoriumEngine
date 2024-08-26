#pragma once

#include "Asset.h"
#include "AudioAsset.generated.h"

// TODO: add classes for each audio file type. (e.g. CMp3Asset, CWavAsset)

ASSET(Abstract)
class ENGINE_API CAudioAsset : public CAsset
{
	GENERATED_BODY()

public:
	void OnInit(IBaseFStream* stream) override;
	void OnLoad(IBaseFStream* stream, uint8) override;

};
