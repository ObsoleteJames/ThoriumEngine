#pragma once

#include "Asset.h"
#include "Skeleton.h"
#include "Math/Transform.h"
#include "Animation.generated.h"

enum EKeyframeType : uint8
{
	KEYFRAME_INVALID	META(Name = "Invalid"),
	KEYFRAME_BONE		META(Name = "Bone"),
	KEYFRAME_PROPERTY	META(Name = "Object Property")
};

enum EKeyframeBehaviour : uint8
{
	KEYFRAME_INTERP_CONSTANT,
	KEYFRAME_INTERP_LINEAR,
};

STRUCT()
struct ENGINE_API FKeyframe
{
	GENERATED_BODY()

public:
	FTransform keyBone;

	// 8 byte variable for object properties
	// a keyframe can only edit integers, bools, and floats.
	SizeType keyProperty; 
};

STRUCT()
struct ENGINE_API FAnimChannel
{
	GENERATED_BODY()

public:
	EKeyframeType type;
	EKeyframeBehaviour behaviour;

	FClass* targetClass = nullptr;

	// Bone/Property name
	// for properties the format is: "variableName", "structName.variableName", "componentName.variableName", e.g. "root.position.x"
	FString targetName;

	// frame - key
	TMap<int, FKeyframe> keyframes;
};

CLASS(Extension = ".thanim", ImportableAs = ".fbx;.gltf;.glb")
class ENGINE_API CAnimation : public CAsset
{
	GENERATED_BODY()

public:
	CAnimation() = default;

	inline float FrameRate() const { return frameRate; }
	inline void SetFrameRate(float f) { frameRate = f; }

	inline int FrameCount() const { return numFrames; }

	// add a new channel, returns null if it already exists
	FAnimChannel* AddChannel(const FString& key);
	FAnimChannel* GetChannel(const FString& key) const;

	void CalculateFrameCount();

protected:
	void OnInit(IBaseFStream* stream) final;

	void OnSave(IBaseFStream* stream) final;
	void OnLoad(IBaseFStream* stream, uint8 lodLevel) final;

private:
	float frameRate = 30.f;
	int numFrames = 0;

	TArray<FAnimChannel> channels;
};
