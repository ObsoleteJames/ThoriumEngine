#pragma once

#include "Object/Object.h"
#include "AudioChannel.generated.h"

CLASS(Abstract)
class IAudioChannel : public CObject
{
	GENERATED_BODY()

public:
	virtual ~IAudioChannel() = default;

};
