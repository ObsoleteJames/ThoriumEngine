#pragma once

#include "Asset.h"
#include "Math/Vectors.h"
#include "Rendering/Shader.h"
#include "Texture.h"
#include "Rendering/Renderer.h"
#include "Material.generated.h"

class IShaderBuffer;

ASSET(Extension = ".thmat")
class ENGINE_API CMaterial : public CAsset
{
	GENERATED_BODY()

public:
	struct MatProperty
	{
		//FShaderProperty* shaderProperty;

		FString name;
		SizeType offset;
		FShaderProperty::EType type;

		bool bRequiresUpdate = false;
		union
		{
			int pBool;
			int pInt;
			float pFloat;
			float pVec3[3];
			float pVec4[4];
		};
	};

	struct MatTexture
	{
		//FShaderTexture* shaderTexture;

		FString name;
		uint8 registerId;

		TObjectPtr<CTexture> tex;
		bool bIsCustom;
		uint8 color[4];
	};

	friend class CMaterialEditor;

public:
	CMaterial() = default;
	virtual ~CMaterial();

	virtual void Init();

	virtual void Save();
	
	// Load textures
	virtual void Load(uint8 lodLevel); 
	virtual void Unload(uint8 lodLevel);

public:
	TObjectPtr<CMaterial> CreateDynamicInstance();

	void SetShader(const FString& shaderName);

	void SetShaderValue(const FString& property, FShaderProperty::EType type, void* data);
	void SetTexture(const FString& name, CTexture* tex);
	void SetTexture(const FString& name, FVector color);

	inline void SetInt(const FString& property, const int& value) { SetShaderValue(property, FShaderProperty::INT, (void*)&value); }
	inline void SetFloat(const FString& property, const float& value) { SetShaderValue(property, FShaderProperty::FLOAT, (void*)&value); }
	inline void SetBool(const FString& property, const bool& value) { SetShaderValue(property, FShaderProperty::BOOL, (void*)&value); }
	inline void SetVec3(const FString& property, float value[3]) { SetShaderValue(property, FShaderProperty::VEC3, value); }
	inline void SetVec4(const FString& property, float value[4]) { SetShaderValue(property, FShaderProperty::VEC4, value); }

	float GetAlpha();
	ERenderPass GetRenderPass();

	void UpdateGpuBuffer();
	void Validate();

	inline CShaderSource* GetShaderSource() const { return shader; }
	inline IShader* GetVsShader() { return shader->vsShader; }
	inline IShader* GetPsShader() { return shader->psShader; }
	inline IShader* GetGeoShader() { return shader->geoShader; }

	inline IShaderBuffer* GetGpuBuffer() const { return gpuBuffer; }

	inline bool DoDepthTest() const { return bDepthTest; }
	inline void EnableDepthTest(bool b) { bDepthTest = b; }

	inline const TArray<MatTexture>& GetTextures() const { return textures; }

public:
	FShaderProperty* GetShaderProperty(MatProperty& prop);
	FShaderTexture* GetShaderTexture(MatTexture& prop);

protected:
	TArray<MatTexture> textures;
	TArray<MatProperty> properties;

	bool bDepthTest = true;

	TObjectPtr<CShaderSource> shader;

	// Buffer for shader properties
	TObjectPtr<IShaderBuffer> gpuBuffer;
};
