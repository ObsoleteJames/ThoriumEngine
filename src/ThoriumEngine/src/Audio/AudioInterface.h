#pragma once

#include "Object/Object.h"
#include "AudioInterface.generated.h"

class IAudioInterface;
class IAudioSource;
class IAudioChannel;
class IAudioSample;
class IAudioStream;

extern ENGINE_API IAudioInterface* gAudioInterface;

class FAudioDevice
{
	int id;
	FString name;
};

enum EAudioOutputType
{
	AUDIO_OUTPUT_STEREO,
	AUDIO_OUTPUT_TV,
	AUDIO_OUTPUT_HEADPHONES,
	AUDIO_OUTPUT_SURROUND_5_1
};

CLASS(Abstract)
class ENGINE_API IAudioInterface : public CObject 
{
	GENERATED_BODY()

public:
	virtual ~IAudioInterface() = default;

	virtual void Init() = 0;
	virtual void Shutdown() = 0;
	virtual void Update() = 0;

	virtual IAudioSource* CreateAudioSource() = 0;
	virtual IAudioSample* CreateAudioSample() = 0;

	virtual IAudioChannel* CreateAudioChannel(const FString& name) = 0;
	virtual IAudioChannel* GetAudioChannel(const FString& name) = 0;

	virtual const FAudioDevice& GetAudioDevice() = 0;
	virtual bool SetAudioDevice(const FAudioDevice& device) = 0;
	virtual bool SetAudioDevice(int id) = 0;
	virtual const TArray<FAudioDevice>& GetAudioDevices() = 0;

};
