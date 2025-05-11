
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

void CAnimation::OnInit(IBaseFStream* stream)
{
	uint numChannels = 0;

	*stream >> &numChannels;
	*stream >> &frameRate;
	*stream >> &numFrames;

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
			int frame;
			
			*stream >> &frame;

			FKeyframe& keyframe = channel->keyframes[frame];
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

	for (auto& ch : channels)
	{
		*stream << &ch.type;
		if (ch.targetClass)
			*stream << ch.targetClass->name;
		else
			*stream << FString();

		*stream << ch.targetName;
		*stream << &ch.behaviour;

		uint frames = ch.keyframes.size();
		*stream << &frames;
		
		for (auto& f : ch.keyframes)
		{
			int frame = f.first;
			*stream << &frame;

			*stream << &f.second.keyBone;
			*stream << &f.second.keyProperty;
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
		for (auto it = ch.keyframes.cbegin(); it != ch.keyframes.cend(); it++)
		{
			if (it->first > numFrames)
				numFrames = it->first;
		}
	}
}
