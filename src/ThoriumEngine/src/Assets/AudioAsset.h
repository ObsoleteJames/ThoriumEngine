#pragma once

#include "Asset.h"
#include "GenericAsset.h"
#include "AudioAsset.generated.h"

// TODO: add classes for each audio file type. (e.g. CMp3Asset, CWavAsset)

ASSET(Abstract)
class ENGINE_API CAudioAsset : public CGenericAsset
{
	GENERATED_BODY()

public:
	/*void OnInit(IBaseFStream* stream) override;
	void OnLoad(IBaseFStream* stream, uint8) override;*/

};

ASSET(Name = "MP3 Asset")
class ENGINE_API CMp3Asset : public CAudioAsset
{
	GENERATED_BODY()

protected:
	void OnLoad(IBaseFStream* stream, uint8 lodLevel) override {}

};

ASSET()
class ENGINE_API CWavAsset : public CAudioAsset
{
	GENERATED_BODY()

protected:
	void OnLoad(IBaseFStream* stream, uint8 lodLevel) override {}
};
