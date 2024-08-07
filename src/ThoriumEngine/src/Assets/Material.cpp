
#include "Material.h"
#include "Rendering/Renderer.h"
#include "Rendering/GraphicsInterface.h"
#include "Rendering/Buffers.h"
#include "TextureAsset.h"
#include "Console.h"
#include "Math/Math.h"

#define THMAT_VERSION_01 1
#define THMAT_VERSION 2

#define THMAT_MAGIC_SIZE 30
static const char* thmatMagicStr = "\0\0ThoriumEngine Material File\0";

CMaterial::~CMaterial()
{
}

void CMaterial::OnInit(IBaseFStream* stream)
{
	if (version != THMAT_VERSION && version != THMAT_VERSION_01)
	{
		CONSOLE_LogError("CMaterial", FString("Invalid Material file version '") + FString::ToString(version) + "'  expected version '" + FString::ToString(THMAT_VERSION) + "'");
		delete stream;
		return;
	}

	properties.Clear();
	textures.Clear();

	FString shaderName;
	*stream >> shaderName;

	shader = CShaderSource::GetShaderSource(shaderName);

	if (version > THMAT_VERSION_01)
	{
		*stream >> &preferredRenderPass;
		*stream >> &bForceTransparentPass;
		*stream >> &bReceiveShadows;
		*stream >> &bCastShadows;
		*stream >> &bDepthTest;
	}

	uint32 numProperties;
	*stream >> &numProperties;

	uint32 numTextures;
	*stream >> &numTextures;

	for (uint32 i = 0; i < numProperties; i++)
	{
		MatProperty p{};

		*stream >> p.name;
		*stream >> &p.type;

		FShaderProperty* shaderProp = GetShaderProperty(p);

		switch (p.type)
		{
		case FShaderProperty::BOOL:
			*stream >> &p.pBool;
			break;
		case FShaderProperty::INT:
			*stream >> &p.pInt;
			break;
		case FShaderProperty::FLOAT:
			*stream >> &p.pFloat;
			break;
		case FShaderProperty::VEC3:
			stream->Read(p.pVec3, sizeof(float) * 3);
			break;
		case FShaderProperty::VEC4:
			stream->Read(p.pVec4, sizeof(float) * 4);
			break;
		}

		if (shaderProp)
		{
			p.offset = shaderProp->offset;
			p.bRequiresUpdate = true;
			properties.Add(p);
		}
	}

	for (uint32 i = 0; i < numTextures; i++)
	{
		MatTexture t{};

		*stream >> t.name;
		*stream >> &t.bIsCustom;

		FShaderTexture* shaderTex = GetShaderTexture(t);

		if (!t.bIsCustom)
		{
			FString texPath;
			*stream >> texPath;

			if (shaderTex)
			{
				t.tex = CAssetManager::GetAsset<CTexture>(texPath);
				if (!t.tex)
					t.tex = CAssetManager::GetAsset<CTexture>("misc/missing.thasset");
			}
		}
		else
		{
			stream->Read(t.color, sizeof(uint8) * 4);

			if (shaderTex)
			{
				uint8 data[] = {
					t.color[0], t.color[1], t.color[2], t.color[3],
					t.color[0], t.color[1], t.color[2], t.color[3],
					t.color[0], t.color[1], t.color[2], t.color[3],
					t.color[0], t.color[1], t.color[2], t.color[3]
				};
				t.tex = CreateObject<CTexture>();
				t.tex->Init(data, 2, 2);
			}
		}

		if (shaderTex)
		{
			t.registerId = shaderTex->registerId;
			textures.Add(t);
		}
	}

	if (shader)
	{
		if (gRenderer)
			gpuBuffer = gGHI->CreateShaderBuffer(nullptr, shader->bufferSize);

		Validate();
		bInitialized = true;
	}
}

void CMaterial::OnSave(IBaseFStream* stream)
{
	*stream << shader->shaderName;

	*stream << &preferredRenderPass;
	*stream << &bForceTransparentPass;
	*stream << &bReceiveShadows;
	*stream << &bCastShadows;
	*stream << &bDepthTest;

	uint32 numProperties = (uint32)properties.Size();
	uint32 numTextures = (uint32)textures.Size();

	*stream << &numProperties << &numTextures;

	for (auto& p : properties)
	{
		*stream << p.name;
		*stream << &p.type;

		switch (p.type)
		{
		case FShaderProperty::BOOL:
			*stream << &p.pBool;
			break;
		case FShaderProperty::INT:
			*stream << &p.pInt;
			break;
		case FShaderProperty::FLOAT:
			*stream << &p.pFloat;
			break;
		case FShaderProperty::VEC3:
			stream->Write(p.pVec3, sizeof(float) * 3);
			break;
		case FShaderProperty::VEC4:
			stream->Write(p.pVec4, sizeof(float) * 4);
			break;
		}
	}

	for (auto& t : textures)
	{
		*stream << t.name;
		*stream << &t.bIsCustom;

		if (!t.bIsCustom)
		{
			if (t.tex)
				*stream << t.tex->GetPath();
			else
				*stream << FString();
		}
		else
		{
			stream->Write(t.color, sizeof(uint8) * 4);
		}
	}
}

void CMaterial::OnLoad(IBaseFStream* stream, uint8 lodLevel)
{
	for (auto& t : textures)
	{
		if (t.tex)
			t.tex->Load(lodLevel);
	}
}

void CMaterial::Unload(uint8 lodLevel)
{
	for (auto& t : textures)
	{
		if (t.tex)
			t.tex->Unload(lodLevel);
	}
}

void CMaterial::LoadTextures(uint8 lod)
{
	for (auto& t : textures)
	{
		if (t.tex)
			t.tex->Load(lod);
	}
}

TObjectPtr<CMaterial> CMaterial::CreateDynamicInstance()
{
	return nullptr;
}

void CMaterial::SetShader(const FString& shaderName)
{
	shader = CShaderSource::GetShaderSource(shaderName);
	if (!shader || shader->type == CShaderSource::ST_INTERNAL)
	{
		CONSOLE_LogError("CMaterial", FString("CMaterial::SetShader - Failed to find shader or shader type was invalid : '") + shaderName + "'!");
		shader = CShaderSource::GetShaderSource("Error");

		if (!shader)
			return;
	}

	if (gRenderer)
		gpuBuffer = gGHI->CreateShaderBuffer(nullptr, shader->bufferSize);

	Validate();

	bInitialized = true;
}

void CMaterial::SetShaderValue(const FString& name, FShaderProperty::EType type, void* data)
{
	MatProperty* property = nullptr;
	for (MatProperty& p : properties)
	{
		if (p.name == name)
		{
			property = &p;
			break;
		}
	}

	if (property)
	{
		if (property->type != type)
			return;

		switch (property->type)
		{
		case FShaderProperty::BOOL:
			property->pBool = *(bool*)data;
			break;
		case FShaderProperty::INT:
			property->pInt = *(int*)data;
			break;
		case FShaderProperty::FLOAT:
			property->pFloat = *(float*)data;
			break;
		case FShaderProperty::VEC3:
			property->pVec3[0] = ((float*)data)[0];
			property->pVec3[1] = ((float*)data)[1];
			property->pVec3[2] = ((float*)data)[2];
			break;
		case FShaderProperty::VEC4:
			property->pVec4[0] = ((float*)data)[0];
			property->pVec4[1] = ((float*)data)[1];
			property->pVec4[2] = ((float*)data)[2];
			property->pVec4[3] = ((float*)data)[3];
			break;
		}
		property->bRequiresUpdate = true;
	}
}

void CMaterial::SetTexture(const FString& name, CTexture* tex)
{
	MatTexture* property = nullptr;
	for (MatTexture& p : textures)
	{
		if (p.name == name)
		{
			property = &p;
			break;
		}
	}
	if (!property)
		return;

	property->bIsCustom = false;
	property->tex = tex;
}

void CMaterial::SetTexture(const FString& name, FVector color)
{
	MatTexture* property = nullptr;
	for (MatTexture& p : textures)
	{
		if (p.name == name)
		{
			property = &p;
			break;
		}
	}
	if (!property)
		return;

	uint8 data[] = {
		uint8(color.x * 255), uint8(color.y * 255), uint8(color.z * 255), 255,
		uint8(color.x * 255), uint8(color.y * 255), uint8(color.z * 255), 255,
		uint8(color.x * 255), uint8(color.y * 255), uint8(color.z * 255), 255,
		uint8(color.x * 255), uint8(color.y * 255), uint8(color.z * 255), 255,
	};
	auto tex = CreateObject<CTexture>();
	tex->Init(data, 2, 2);

	property->bIsCustom = true;
	property->tex = tex;
}

float CMaterial::GetAlpha()
{
	auto it = properties.Find([](const CMaterial::MatProperty& p) { return p.name == "vAlpha"; });
	if (it != properties.end())
		return it->pFloat;
	return 1.f;
}

ERenderPass CMaterial::GetRenderPass()
{
	uint8 matType = GetShaderSource()->type;
	if (matType == CShaderSource::ST_DEFERRED)
		return R_DEFERRED_PASS;

	if (matType == CShaderSource::ST_FORWARD_DEFERRED)
	{
#if ENABLE_DEFERRED_RENDERING == 0
		// for now just use forward pass until deferred rendering is working.
		return (GetAlpha() < 1.f || bForceTransparentPass) ? R_TRANSPARENT_PASS : R_FORWARD_PASS;
#endif
		if (cvForceForwardRendering.AsBool())
			return (GetAlpha() < 1.f || bForceTransparentPass) ? R_TRANSPARENT_PASS : R_FORWARD_PASS;

		if (preferredRenderPass == PRP_FORWARD)
			return (GetAlpha() < 1.f || bForceTransparentPass) ? R_TRANSPARENT_PASS : R_FORWARD_PASS;

		if (preferredRenderPass == PRP_DEFERRED)
			return R_DEFERRED_PASS;

		if (preferredRenderPass == PRP_OPAQUE_DEFERRED_TRANSPARENT_FORWARD)
			return (GetAlpha() < 1.f || bForceTransparentPass) ? R_TRANSPARENT_PASS : R_DEFERRED_PASS;
	}

	if (matType == CShaderSource::ST_DEBUG) {
		if (DoDepthTest())
			return R_DEBUG_PASS;
		else
			return R_DEBUG_OVERLAY_PASS;
	}
	
	if (matType == CShaderSource::ST_FORWARD)
		return (GetAlpha() < 1.f || bForceTransparentPass) ? R_TRANSPARENT_PASS : R_FORWARD_PASS;

	return RENDER_PASS_NONE;
}

void CMaterial::UpdateGpuBuffer()
{
	bool bUpdate = false;
	for (auto& p : properties)
	{
		if (p.bRequiresUpdate)
		{
			p.bRequiresUpdate = false;
			bUpdate = true;
			continue;
		}
	}

	if (!bUpdate)
		return;

	char* buff = (char*)malloc(shader->bufferSize);

	for (auto& p : properties)
	{
		switch (p.type)
		{
		case FShaderProperty::BOOL:
			memcpy(buff + p.offset, &p.pBool, sizeof(int));
			break;
		case FShaderProperty::INT:
			memcpy(buff + p.offset, &p.pInt, sizeof(int));
			break;
		case FShaderProperty::FLOAT:
			memcpy(buff + p.offset, &p.pFloat, sizeof(float));
			break;
		case FShaderProperty::VEC3:
			memcpy(buff + p.offset, p.pVec3, sizeof(p.pVec3));
			break;
		case FShaderProperty::VEC4:
			memcpy(buff + p.offset, p.pVec4, sizeof(p.pVec4));
			break;
		}
	}

	gpuBuffer->Update(shader->bufferSize, buff);
	free(buff);
}

void CMaterial::Validate()
{
	shader->LoadShaderObjects();

	// First check for invalid properties
	for (auto it = properties.rbegin(); it != properties.rend(); it++)
	{
		bool bExists = GetShaderProperty(*it) != nullptr;
		
		if (!bExists)
			properties.Erase(it);
	}
	for (auto it = textures.rbegin(); it != textures.rend(); it++)
	{
		bool bExists = GetShaderTexture(*it) != nullptr;

		if (!bExists)
			textures.Erase(it);
	}

	// Then add new properties
	for (auto& sp : shader->properties)
	{
		bool bExists = false;
		for (auto& p : properties)
		{
			if (p.name == sp.name && p.type == sp.type)
			{
				bExists = true;
				p.offset = sp.offset;
				break;
			}
		}

		if (!bExists)
		{
			MatProperty p{};
			p.name = sp.name;
			p.type = sp.type;
			p.offset = sp.offset;

			char data[4]{ 0 };
			switch (sp.type)
			{
			case FShaderProperty::BOOL:
				if (sp.initValue == "true")
					p.pBool = true;
				else if (sp.initValue == "false")
					p.pBool = false;
				else
					p.pBool = *(bool*)data;
				break;
			case FShaderProperty::INT:
				if (sp.initValue.IsNumber() && !sp.initValue.IsEmpty())
					p.pInt = std::stoi(sp.initValue.c_str());
				else
					p.pInt = *(int*)data;
				break;
			case FShaderProperty::FLOAT:
				if (sp.initValue.IsNumber() && !sp.initValue.IsEmpty())
					p.pFloat = std::stof(sp.initValue.c_str());
				else
					p.pFloat = *(float*)data;
				break;
			case FShaderProperty::VEC3:
				if (sp.initValue.IsEmpty())
				{
					p.pVec3[0] = *(float*)data;
					p.pVec3[1] = *(float*)data;
					p.pVec3[2] = *(float*)data;
				}
				else
				{
					FVector v = GetVec3FromString(sp.initValue);
					p.pVec3[0] = v.x;
					p.pVec3[1] = v.y;
					p.pVec3[2] = v.z;
				}
				break;
			case FShaderProperty::VEC4:
				if (sp.initValue.IsEmpty())
				{
					p.pVec4[0] = *(float*)data;
					p.pVec4[1] = *(float*)data;
					p.pVec4[2] = *(float*)data;
					p.pVec4[3] = *(float*)data;
				}
				else
					GetVec4FromString(sp.initValue, p.pVec4);
				break;
			}

			properties.Add(p);
		}
	}
	for (auto& st : shader->textures)
	{
		bool bExists = false;
		for (auto& p : textures)
		{
			if (p.name == st.name && p.registerId == st.registerId)
			{
				bExists = true;
				break;
			}
		}

		if (!bExists)
		{
			MatTexture t{};
			t.name = st.name;
			t.registerId = st.registerId;
			t.tex = nullptr;

			if (st.initValue.IsEmpty())
			{
				if (auto i = st.initValue.Find("Color("); i != -1)
				{
					t.bIsCustom = true;

					float color[4];
					FString v = st.initValue;
					v.Erase(v.begin(), v.begin() + 6);

					GetVec4FromString(v, color);

					t.color[0] = uint8(color[0] * 255.f);
					t.color[1] = uint8(color[1] * 255.f);
					t.color[2] = uint8(color[2] * 255.f);
					t.color[3] = uint8(color[3] * 255.f);

					uint8 data[] = {
						t.color[0], t.color[1], t.color[2], t.color[3],
						t.color[0], t.color[1], t.color[2], t.color[3],
						t.color[0], t.color[1], t.color[2], t.color[3],
						t.color[0], t.color[1], t.color[2], t.color[3]
					};

					t.tex = CreateObject<CTexture>();
					t.tex->Init(data, 2, 2);
				}
				else if (!st.initValue.IsEmpty())// else this must be an asset path.
				{
					t.bIsCustom = false;
					t.tex = CAssetManager::GetAsset<CTexture>(st.initValue);
				}
			}

			textures.Add(t);
		}
	}

	SetFloat("vAlpha", 1.f);
}

FShaderProperty* CMaterial::GetShaderProperty(MatProperty& prop)
{
	if (!shader)
		return nullptr;

	for (auto& sp : shader->properties)
	{
		if (sp.name == prop.name && sp.type == prop.type)
			return &sp;
	}

	return nullptr;
}

FShaderTexture* CMaterial::GetShaderTexture(MatTexture& prop)
{
	if (!shader)
		return nullptr;

	for (auto& sp : shader->textures)
	{
		if (sp.name == prop.name)
			return &sp;
	}

	return nullptr;
}

uint8 CMaterial::GetFileVersion() const
{
	return THMAT_VERSION;
}

FVector CMaterial::GetVec3FromString(const FString& str)
{
	FVector r;
	int chI = 0;
	for (int i = 0; i < 3; i++)
	{
		FString value;
		for (; chI < str.Size(); chI++)
		{
			if (str[chI] == '{' || str[chI] == '(')
				continue;
			if (str[chI] == '}' || str[chI] == ')')
				break;

			if (str[chI] == ' ' || str[chI] == '\t')
				continue;

			if (str[chI] == ',')
			{
				chI++;
				break;
			}

			value += str[chI];
		}

		if (!value.IsNumber())
			continue;

		switch (i)
		{
		case 0:
			r.x = std::stof(value.c_str());
			break;
		case 1:
			r.y = std::stof(value.c_str());
			break;
		case 2:
			r.z = std::stof(value.c_str());
			break;
		}
	}
	return r;
}

void CMaterial::GetVec4FromString(const FString& str, float* out)
{
	int chI = 0;
	for (int i = 0; i < 4; i++)
	{
		FString value;
		for (; chI < str.Size(); chI++)
		{
			if (str[chI] == '{' || str[chI] == '(')
				continue;
			if (str[chI] == '}' || str[chI] == ')')
				break;

			if (str[chI] == ' ' || str[chI] == '\t')
				continue;

			if (str[chI] == ',')
			{
				chI++;
				break;
			}

			value += str[chI];
		}

		if (value.IsEmpty() || !value.IsNumber())
			out[i] = 0.f;
		else
			out[i] = std::stof(value.c_str());
	}
}
