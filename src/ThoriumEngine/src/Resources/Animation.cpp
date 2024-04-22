
#include "Animation.h"

void CAnimation::Init()
{

}

void CAnimation::Save()
{

}

void CAnimation::Load(uint8 lodLevel)
{

}

TArray<FKeyframe>* CAnimation::GetKeyframes(int frame) const
{
	if (frame < 0 || frame > numFrames)
		return 0;

	auto it = frames.find(frame);
	if (it == frames.end())
		return nullptr;

	return (TArray<FKeyframe>*)&(it->second);
}

int CAnimation::GetNextFrame(int index)
{
	if (index < frameIndex.Size())
		return frameIndex[index].Value;

	return -1;
}

int CAnimation::GetPreviousFrame(int index)
{
	if (index < frameIndex.Size())
		return frameIndex[index].Key;

	return -1;
}

bool CAnimation::AddKeyframe(int frame, const FKeyframe& keyframe)
{
	frames[frame].Add(keyframe);

	GenerateFrameIndex();
	return true;
}

void CAnimation::GenerateFrameIndex()
{
	if (frames.size() == 0)
		return;

	int lastFrame = frames.rbegin()->first;
	numFrames = lastFrame;

	frameIndex.Resize(lastFrame);
	for (int i = 0; i < lastFrame; i++)
	{
		frameIndex[i].Key = SearchPrevious(i);
		frameIndex[i].Value = SearchNext(i);
	}
}

int CAnimation::SearchNext(int index)
{
	auto it = frames.find(index);
	if (it != frames.end())
		return index;

	while (it == frames.end())
	{
		index++;
		if (index > numFrames)
			return -1;

		it = frames.find(index);
	}
	return index;
}

int CAnimation::SearchPrevious(int index)
{
	index--;
	auto it = frames.find(index);
	if (it != frames.end())
		return index;

	while (it == frames.end())
	{
		index--;
		if (index < 0)
			return -1;

		it = frames.find(index);
	}
	return index;
}
