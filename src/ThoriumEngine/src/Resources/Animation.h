#pragma once

#include "Asset.h"
#include "Skeleton.h"
#include "Math/Transform.h"
#include "Animation.generated.h"

ENUM()
enum EKeyframeType
{
	KEYFRAME_INVALID	META(Name = "Invalid"),
	KEYFRAME_BONE		META(Name = "Bone"),
	KEYFRAME_PROPERTY	META(Name = "Object Property")
};

STRUCT()
struct ENGINE_API FKeyframe
{
	GENERATED_BODY()

public:
	EKeyframeType type;

	FClass* targetClass = nullptr;
	
	// Bone/Property name
	// for properties the format is: "variableName", "structName.variableName", "componentName.variableName", e.g. "root.position.x"
	FString targetName;

	int frame;

	union
	{
		FTransform keyBone;

		// 8 byte variable for object properties
		// a keyframe can only edit integers, bools, and floats.
		SizeType keyProperty; 
	};
};

CLASS(Extension = ".thanim", ImportableAs = ".fbx;.gltf;.glb")
class ENGINE_API CAnimation : public CAsset
{
	GENERATED_BODY()

public:
	CAnimation() = default;

	void Init() final;

	void Save() final;
	void Load(uint8 lodLevel) final;

	inline float FrameRate() const { return frameRate; }
	inline void SetFrameRate(float f) { frameRate = f; }

	inline int FrameCount() const { return numFrames; }

	TArray<FKeyframe>* GetKeyframes(int frame) const;

	int GetNextFrame(int index);
	int GetPreviousFrame(int index);

	bool AddKeyframe(int frame, const FKeyframe& keyframe);

private:
	void GenerateFrameIndex();

	int SearchNext(int index);
	int SearchPrevious(int index);

private:
	float frameRate = 30.f;
	int numFrames = 0;

	// frame index - keyframes
	TMap<int, TArray<FKeyframe>> frames;

	// for quick searching of next/previous frames.
	// if the index is on a frame, the next value will be that frame.
	// key = previous, value = next.
	TArray<TPair<int, int>> frameIndex;
};
