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
struct FKeyframe
{
	GENERATED_BODY()

public:
	EKeyframeType type;

	FClass* targetClass = nullptr;
	FString targetName; // Bone/Property name

	int frame;

	union
	{
		FTransform keyBone;
		float keyProperty[16]; // 64 byte structure for object properties
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

private:
	float frameRate = 30.f;
	int numFrames = 0;

	// frame index - keyframes
	TMap<int, TArray<FKeyframe>> frames;
};
