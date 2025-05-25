#pragma once

#include "Object/Object.h"
#include "Assets/Animation.h"
#include "Assets/Skeleton.h"
#include "Assets/ModelAsset.h"

// base animation proxy class
class ENGINE_API IAnimationProxy
{
public:
	IAnimationProxy() = default;
	virtual ~IAnimationProxy() = default;

	virtual void Update(double dt) = 0;

	virtual void SetFloat(const FString& key, float value) {}
	virtual void SetInt(const FString& key, int value) {}
	virtual void SetBool(const FString& key, bool value) {}
	virtual void SetVector(const FString& key, const FVector& value) {}

	inline float CurrentFrame() const { return frame; }
	inline float FrameRate() const { return framerate; }

protected:
	TObjectPtr<CObject> owner = nullptr;

	float frame = 0.f;
	float framerate = 30.f;
};

// basic animation proxy for a single animation
class ENGINE_API CAnimationProxy : public IAnimationProxy
{
public:
	CAnimationProxy(CObject* owner, CModelAsset* model, FSkeletonInstance* skeleton, CAnimation* anim);

	virtual void Update(double dt) override;

private:
	void BuildSkeletonLUT();

private:
	FSkeletonInstance* skeleton;
	CAnimation* animfile;

	CModelAsset* mdl;

	// channel index - bone index
	TMap<int, int> skeletonLut;
};
