#pragma once

#include "Object/Object.h"
#include "AudioSource.generated.h"

class IAudioChannel;
class IAudioSample;
struct FVector;

CLASS(Abstract)
class IAudioSource : public CObject
{
	GENERATED_BODY()

public:
	virtual ~IAudioSource() = default;
	
	virtual void SetSample(IAudioSample* sample) = 0;

	virtual void Play() = 0;

	virtual void SetChannel(IAudioChannel*) = 0;
	virtual void SetLooping(bool) = 0;
	virtual void SetVolume(float) = 0;
	virtual void SetPitch(float) = 0;
	virtual void SetPanning(float) = 0;
	virtual void SetPosition(FVector* target, bool bUpdatePosition = true);

protected:
	// wether to keep updating the source position.
	bool bUpdatePos = true;
	FVector* position = nullptr;
};
