#pragma once

#include "EngineCore.h"
#include "Math/Vectors.h"
#include "Object/Object.h"
#include "Assets/Asset.h"
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
	ShaderFeature_None = 0,
	ShaderFeature_Lighting = 1,
	ShaderFeature_CubeMapLighting = 1 << 1,
	ShaderFeature_Shadows = 1 << 2, // allows shadows to be used by this shader (forward only)
};

enum EShaderType
{
	ShaderType_Vertex = 1,
	ShaderType_Fragment = 1 << 1,
	ShaderType_Geometry = 1 << 2,
	ShaderType_ForwardPass = 1 << 3,
	ShaderType_DeferredPass = 1 << 4
};
typedef int EShaderType_;

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
		ST_FORWARD_DEFERRED // Has both forward and deferred shaders
	};

public:
	CShaderSource() = default;
	virtual ~CShaderSource();

	virtual void OnInit(IBaseFStream* stream);

	virtual void OnSave(IBaseFStream* stream);
	virtual void OnLoad(IBaseFStream* stream, uint8 lodLevel) {}

	void OnDelete() override;

	bool Compile();

	void LoadShaderObjects();

	static CShaderSource* GetShaderSource(const FString& name);
	static const TArray<TObjectPtr<CShaderSource>>& GetAllShaders();

	inline bool HasFeature(EShaderFeatures feature) const { return (features & feature) != 0; }

	IShader* GetShader(EShaderType_ type);

	static FString GetShaderName(EShaderType_ type);

private:
	void LoadVersion05(IBaseFStream* stream);

public:
	TArray<FShaderProperty> properties;
	TArray<FShaderTexture> textures;
	FString shaderName;
	FString description;
	int8 type; // CShaderSource::EType

	uint16 version;

	uint8 features = ShaderFeature_None;

	SizeType bufferSize;

	TArray<TPair<uint8, IShader*>> shaders;

	//IShader* vsShader = nullptr;
	//IShader* psShader = nullptr;
	//IShader* geoShader = nullptr;

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
