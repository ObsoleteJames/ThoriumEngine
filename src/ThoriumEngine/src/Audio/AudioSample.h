#pragma once

#include "Object/Object.h"
#include "AudioSample.generated.h"

class IAudioChannel;

CLASS(Abstract)
class IAudioSample : public CObject
{
	GENERATED_BODY()

public:
	virtual ~IAudioSample() = default;
	
	virtual void LoadFromFile(const FString& file) = 0;

	virtual void Play(float volume = 1.0f, float panning = 0.f, float pitch = 1.0) = 0;

};
