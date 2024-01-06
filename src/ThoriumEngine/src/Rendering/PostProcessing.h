#pragma once

#include "Object/Object.h"
#include "Math/Vectors.h"
#include "Math/Bounds.h"
#include "PostProcessing.generated.h"

class CMaterial;
class CTexture;

ENUM()
enum EExposureType
{
	ExposureType_Constant	META(Name = "Constant"),
	ExposureType_Dynamic	META(Name = "Dynamic"),
};

ENUM()
enum EPostProcessFeature
{
	PPF_Exposure		META(Name = "Exposure"),
	PPF_ColorGrading	META(Name = "Color Grading"),
	PPF_Bloom			META(Name = "Bloom"),
};

STRUCT()
struct FPostProcessSettings
{
	GENERATED_BODY()

public:
	// ------------- Bloom -------------
	PROPERTY(Name = "Intensity", Category = "Bloom")
	float bloomIntensity = 0.5f;

	PROPERTY(Name = "Threshold", Category = "Bloom")
	float bloomThreshold = 1.f;

	PROPERTY(Name = "Blur Amount", Category = "Bloom")
	float bloomBlur = 2.5f;

	// ------------- Exposure -------------
	PROPERTY(Category = "Exposure")
	EExposureType exposureType;

	PROPERTY(Name = "Exposure", Category = "Exposure")
	float exposureIntensity = 1.f;

	PROPERTY(Name = "Dynamic Min", Category = "Exposure")
	float exposureDynamicMin;

	PROPERTY(Name = "Dynamic Max", Category = "Exposure")
	float exposureDynamicMax;

	// ------------- Color Grading/Correction -------------
	PROPERTY(Category = "Color Grading")
	float gamma = 2.2f;

	PROPERTY(Name = "Color Grading LUT", Category = "Color Grading")
	TObjectPtr<CTexture> gradingLut = nullptr;
};

class ENGINE_API CPostProcessVolumeProxy
{
	friend class IRenderer;

public:
	CPostProcessVolumeProxy() = default;
	virtual ~CPostProcessVolumeProxy() = default;

	virtual void FetchData() = 0;

	inline const FPostProcessSettings& PostProcessSettings() const { return ppSettings; }

	inline const FBounds& Bounds() const { return bounds; }
	inline const FQuaternion& Rotation() const { return rotation; }

	inline float Fade() const { return fade; }
	
	inline bool IsGlobal() const { return bGlobal; }
	inline bool IsEnabled() const { return bEnabled; }

	bool IsCameraInsideVolume(CCameraProxy* camera) const;
	float GetInfluence(CCameraProxy* camera) const;

protected:
	// if set, this volume will apply this material to the screen, instead of the default Post Processing features.
	TObjectPtr<CMaterial> postProcessMaterial = nullptr;

	FPostProcessSettings ppSettings;

	bool bGlobal;
	bool bEnabled;
	float fade;

	FQuaternion rotation;
	FBounds bounds; // object oriented bounds.
};
