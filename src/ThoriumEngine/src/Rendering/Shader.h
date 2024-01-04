#pragma once

#include "EngineCore.h"
#include "Math/Vectors.h"
#include "Object/Object.h"
#include "Resources/Asset.h"
#include "Shader.generated.h"

struct FFile;
class IShader;
class IShaderBuffer;

struct ENGINE_API FShaderProperty
{
	enum EType
	{
		NONE,
		BOOL,
		INT,
		FLOAT,
		VEC3,
		VEC4,
		TEXTURE_2D,
		TEXTURE_CUBE
	};

	enum UiType
	{
		BOX,
		SLIDER,
		COLOR,
	};

	FString name;
	FString displayName;
	FString description;
	FString UiGroup;
	FString initValue;

	EType type;
	UiType uiType;
	SizeType offset;
};

struct ENGINE_API FShaderTexture
{
	FString name;
	FString displayName;
	uint8 registerId;
	FString UiGroup;
	FString initValue;
};

enum EShaderFeatures
{
	ShaderFeature_Lighting = 1,
	ShaderFeature_CubeMapLighting = 1 << 1
};

ASSET(Extension = ".thcs", Hidden, AutoLoad)
class ENGINE_API CShaderSource : public CAsset
{
	GENERATED_BODY()

public:
	enum EType : int8
	{
		ST_UNKNOWN,
		ST_FORWARD,
		ST_DEFERRED,
		ST_POSTPROCESS,
		ST_DEBUG,
		ST_INTERNAL,
	};

public:
	CShaderSource() = default;
	virtual ~CShaderSource();

	virtual void Init();

	virtual void Save();
	virtual void Load(uint8 lodLevel) {}

	void OnDelete() override;

	bool Compile();

	void LoadShaderObjects();

	static CShaderSource* GetShader(const FString& name);
	static const TArray<TObjectPtr<CShaderSource>>& GetAllShaders();

private:
	void LoadVersion05(IBaseFStream* stream);

public:
	TArray<FShaderProperty> properties;
	TArray<FShaderTexture> textures;
	FString shaderName;
	FString description;
	int8 type; // FShaderSourceFile::EType

	SizeType bufferSize;

	IShader* vsShader;
	IShader* psShader;
	IShader* geoShader;

	int8 bCompiled;
	int8 bHasPS;
	int8 bHasVS;
	int8 bHasGEO;
};

class ENGINE_API IShader
{
public:
	enum EType
	{
		VFX_VS,
		VFX_PS,
		VFX_GEO, 
		HLSL_CUSTOM
	};

public:
	virtual ~IShader() = default;

	inline CShaderSource* GetSource() const { return shaderSource; }
	inline bool IsValid() const { return bValid; }

protected:
	int8 type;
	bool bValid;

	CShaderSource* shaderSource;
};
