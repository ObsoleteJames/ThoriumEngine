
#include "AnimationProxy.h"

CAnimationProxy::CAnimationProxy(CObject* o, CModelAsset* model, FSkeletonInstance* sk, CAnimation* anim)
{
	owner = o;
	animfile = anim;

	framerate = anim->FrameRate();

	mdl = model;
	skeleton = sk;
	BuildSkeletonLUT();
}

void CAnimationProxy::Update(double dt)
{
	frame += dt * animfile->FrameRate();

	float frameDelta = FMath::Mod(frame, 1.f);

	int frame1 = (int)FMath::Floor(frame) % animfile->FrameCount();
	int frame2 = (int)FMath::Ceil(frame) % animfile->FrameCount();

	for (int i = 0; i < animfile->NumChannels(); i++)
	{
		const FAnimChannel* channel = animfile->GetChannel(i);
		if (channel->type != KEYFRAME_BONE)
			continue;

		if (skeletonLut[i] == -1)
			continue;

		if (channel->behaviour == KEYFRAME_INTERP_LINEAR)
		{
			const FKeyframe& key1 = channel->keyframes.lower_bound(frame1 % channel->keyframes.size())->second;
			const FKeyframe& key2 = channel->keyframes.lower_bound(frame2 % channel->keyframes.size())->second;
			
			FTransform boneT = FTransform::Lerp(key1.keyBone, key2.keyBone, frameDelta);
			boneT.position = FVector::zero;
			boneT.rotation = boneT.rotation.Conjugate();
			skeleton->bones[skeletonLut[i]] = boneT;
		}
		else
			skeleton->bones[skeletonLut[i]] = channel->keyframes.at(frame1).keyBone;
	}

	// loop animation
	if (frame > animfile->FrameCount())
		frame -= (float)animfile->FrameCount();
}

void CAnimationProxy::BuildSkeletonLUT()
{
	skeletonLut.clear();

	for (int i = 0; i < animfile->NumChannels(); i++)
	{
		const FAnimChannel* channel = animfile->GetChannel(i);

		if (channel->type != KEYFRAME_BONE)
			continue;

		int bone = -1;

		for (int b = 0; b < skeleton->bones.Size(); b++)
		{
			if (mdl->GetSkeleton().bones[b].name == channel->targetName)
			{
				bone = b;
				break;
			}
		}

		skeletonLut[i] = bone;
	}
}
