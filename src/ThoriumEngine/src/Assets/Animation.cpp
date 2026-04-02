
#include "Animation.h"

FAnimChannel* CAnimation::AddChannel(const FString& key)
{
	for (auto& ch : channels)
		if (ch.targetName == key)
			return nullptr;

	channels.Add();
	channels.last()->targetName = key;
	return &*channels.last();
}

FAnimChannel* CAnimation::GetChannel(const FString& key) const
{
	for (auto& ch : channels)
		if (ch.targetName == key)
			return &ch;

	return nullptr;
}

void CAnimation::ClearChannels()
{
	channels.Clear();
}

int CAnimation::GetKeyframeIndex(const FAnimChannel* channel, float time)
{
	for (int i = 0; i < channel->keyframes.Size(); i++)
	{
		if (i + 1 < channel->keyframes.Size())
		{
			if (time >= channel->keyframes[i].time && time < channel->keyframes[i + 1].time)
				return i;
		}
		else if (time >= channel->keyframes[i].time)
			return i;
	}

	return 0;
}

int CAnimation::GetNextKeyframeIndex(const FAnimChannel* channel, float time)
{
	for (int i = 0; i < channel->keyframes.Size(); i++)
	{
		if (time >= channel->keyframes[i].time)
		{
			i++;
			if (i >= channel->keyframes.Size())
				return 0;
			return i;
		}
	}

	return 0;
}

int CAnimation::GetNextKeyframeIndex(const FAnimChannel* channel, int prevIndex)
{
	prevIndex++;
	if (prevIndex >= channel->keyframes.Size())
		return 0;
	return prevIndex;
}

void CAnimation::OnInit(IBaseFStream* stream)
{
	uint numChannels = 0;

	*stream >> &numChannels;
	*stream >> &frameRate;
	*stream >> &numFrames;
	*stream >> &length;

	for (int i = 0; i < numChannels; i++)
	{
		FString targetClass;
		FString targetName;
		EKeyframeType type;
		EKeyframeBehaviour behaviour;

		*stream >> &type;
		*stream >> targetClass;
		*stream >> targetName;
		*stream >> &behaviour;

		FAnimChannel* channel = AddChannel(targetName);
		if (!targetClass.IsEmpty())
			channel->targetClass = CModuleManager::FindClass(targetClass);

		channel->type = type;
		channel->behaviour = behaviour;

		uint frames = 0;
		*stream >> &frames;

		for (int ii = 0; ii < frames; ii++)
		{
			channel->keyframes.Add();
			FKeyframe& keyframe = *channel->keyframes.last();
			*stream >> &keyframe.time;
			*stream >> &keyframe.keyBone;
			*stream >> &keyframe.keyProperty;
		}
	}
}

void CAnimation::OnSave(IBaseFStream* stream)
{
	CalculateFrameCount();

	uint numChannels = channels.Size();
	*stream << &numChannels;
	*stream << &frameRate;
	*stream << &numFrames;
	*stream << &length;

	for (auto& ch : channels)
	{
		*stream << &ch.type;
		if (ch.targetClass)
			*stream << ch.targetClass->name;
		else
			*stream << FString();

		*stream << ch.targetName;
		*stream << &ch.behaviour;

		uint frames = ch.keyframes.Size();
		*stream << &frames;
		
		for (auto& f : ch.keyframes)
		{
			*stream << &f.time;

			*stream << &f.keyBone;
			*stream << &f.keyProperty;
		}
	}
}

// unused - data gets loaded when initialized
void CAnimation::OnLoad(IBaseFStream* stream, uint8 lodLevel)
{
}

void CAnimation::CalculateFrameCount()
{
	numFrames = 0;
	for (auto& ch : channels)
	{
		if (ch.keyframes.Size() > numFrames)
			numFrames = ch.keyframes.Size();
	}

	length = 0;
	for (auto& ch : channels)
	{
		for (auto it = ch.keyframes.begin(); it != ch.keyframes.end(); it++)
		{
			if (it->time > length)
				length = it->time;
		}
	}
}
