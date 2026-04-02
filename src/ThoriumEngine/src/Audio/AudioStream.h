#pragma once

#include "Object/Object.h"
#include "AudioStream.generated.h"

class IAudioChannel;

CLASS(Abstract)
class IAudioStream : public CObject
{
	GENERATED_BODY()

public:
	virtual ~IAudioStream() = default;

};
