
#include "ShaderParser.h"
#include "Renderer.h"
#include "GraphicsInterface.h"
#include "Console.h"

#include <Util/FStream.h>

#define THCS_VERSION_06 6
#define THCS_VERSION_07 7
#define THCS_VERSION 8

#define THCS_MAGIC_SIZE 28
static const char* thcsMagicStr = "\0\0ThoriumEngine Shader File\0";

static TArray<TObjectPtr<CShaderSource>> _shaders;

static CConCmd cmdDebugShader("shader.printinfo", [](const TArray<FString>& args) {
	if (args.Size() == 0)
		return;
	
	CShaderSource* shader = CShaderSource::GetShaderSource(args[0]);
	if (!shader)
	{
		CONSOLE_LogWarning("CShaderSource", "Unknown shader: " + args[0]);
		return;
	}

	CConsole::LogPlain("Shader: " + shader->shaderName);
	CConsole::LogPlain("BufferSize: " + FString::ToString(shader->bufferSize));

	CConsole::LogPlain("properties:");
	CConsole::LogPlain("    PropertyName  -  DisplayName  -  Type  -  DataOffset");
	for (auto& prop : shader->properties)
	{
		constexpr const char* typeNames[] = {
			"NONE",
			"BOOL",
			"INT",
			"FLOAT",
			"VEC3",
			"VEC4",
			"TEXTURE_2D",
			"TEXTURE_CUBE"
		};

		FString propertyTypeName = typeNames[prop.type];
		
		CConsole::LogPlain("    " + prop.name + ":    " + prop.displayName + "    " + propertyTypeName + "    " + FString::ToString(prop.offset));
	}
	CConsole::LogPlain("textures:");
	CConsole::LogPlain("    TextureName  -  DisplayName  -  Register");
	for (auto& tex : shader->textures)
	{
		CConsole::LogPlain("    " + tex.name + ":    " + tex.displayName + "    " + FString::ToString(tex.registerId));
	}
});

CShaderSource::~CShaderSource()
{
	for (auto& sh : shaders)
		delete sh.Value;
	shaders.Clear();
}

void CShaderSource::OnInit(IBaseFStream* stream)
{
	// Add the static properties and textures.
	//properties.Add({ "vColorTint", "Color Tint", "", "Color", FShaderProperty::VEC4, FShaderProperty::COLOR, 0});
	//properties.Add({ "vNormalIntensity", "Normal Map Strength", "", "Normal", FShaderProperty::FLOAT, FShaderProperty::SLIDER, 16 });
	//properties.Add({ "vAlpha", "Alpha", "", "Color", FShaderProperty::FLOAT, FShaderProperty::SLIDER, 20});

	//textures.Add({ "vBaseColor", "Base Color", 5, "Color" });
	//textures.Add({ "vNormalMap", "Normal Map", 6, "Normal" });

	// Register the shader
	_shaders.Add(this);

	char magicStr[THCS_MAGIC_SIZE];
	stream->Read(magicStr, THCS_MAGIC_SIZE);
	if (memcmp(thcsMagicStr, magicStr, THCS_MAGIC_SIZE) != 0)
	{
		// Check if this file isn't empty, if it is, try to compile it.
		if (stream->Tell() == 0)
		{
			stream = nullptr;
			if (gRenderer && Compile())
				bInitialized = true;
		}
		else
			CONSOLE_LogError("CShaderSource", FString("Invalid Shader file '") + ToFString(file->Path()) + "'");
		return;
	}

	bCompiled = true;

	if (version != THCS_VERSION && version != THCS_VERSION_06 && version != THCS_VERSION_07)
	{
		if (version == 5)
		{
			LoadVersion05(stream);
			return;
		}
		CONSOLE_LogError("CShaderSource", FString("Invalid Shader file version '") + FString::ToString(version) + "'  expected version '" + FString::ToString(THCS_VERSION) + "'");
		return;
	}

	*stream >> shaderName;
	*stream >> description;
	*stream >> &type;
	*stream >> &bufferSize;

	if (version > THCS_VERSION_07)
	{
		uint8 numShaders;
		*stream >> &numShaders;

		for (uint8 i = 0; i < numShaders; i++)
		{
			// Shader type
			shaders.Add();
			*stream >> &shaders.last()->Key;
			shaders.last()->Value = nullptr;
		}
	}

	if (version <= THCS_VERSION_07)
	{
		*stream >> &bHasVS >> &bHasPS >> &bHasGEO;

		if (bHasVS)
			shaders.Add({ ShaderType_Vertex | ShaderType_ForwardPass | ShaderType_DeferredPass, nullptr });
		if (bHasPS)
			shaders.Add({ ShaderType_Fragment | ShaderType_ForwardPass | ShaderType_DeferredPass, nullptr });
		if (bHasGEO)
			shaders.Add({ ShaderType_Geometry | ShaderType_ForwardPass | ShaderType_DeferredPass, nullptr });
	}

	uint32 numProperties = 0;
	*stream >> &numProperties;

	for (uint32 i = 0; i < numProperties; i++)
	{
		FString pName;
		FString pDisplayName;
		FString pDescription;
		FShaderProperty::EType pType;
		FString pUiGroup;
		FShaderProperty::UiType pUiType;
		SizeType pBufferOffset;
		FString initValue;

		*stream >> pName;
		*stream >> pDisplayName;
		*stream >> pDescription;
		*stream >> &pType;
		*stream >> pUiGroup;
		*stream >> &pUiType;
		*stream >> &pBufferOffset;
		if (version > THCS_VERSION_06)
			*stream >> initValue;

		if (pType > FShaderProperty::VEC4)
		{
			FShaderTexture tex{ pName, pDisplayName, (uint8)pBufferOffset, pUiGroup, initValue };
			textures.Add(tex);
		}
		else
		{
			FShaderProperty p{ pName, pDisplayName, pDescription, pUiGroup, initValue, (FShaderProperty::EType)pType, (FShaderProperty::UiType)pUiType, pBufferOffset };
			properties.Add(p);
		}
	}

	bInitialized = true;
}

void CShaderSource::OnSave(IBaseFStream* stream)
{
	stream->Write((void*)thcsMagicStr, THCS_MAGIC_SIZE);

	*stream << shaderName;
	*stream << description;
	*stream << (int8*)&type;
	*stream << &bufferSize;

	uint8 numShaders = (uint8)shaders.Size();
	*stream << &numShaders;

	for (uint8 i = 0; i < numShaders; i++)
	{
		// Shader type
		*stream << &shaders[i].Key;
	}

	uint32 numProperties = (uint32)properties.Size() + (uint32)textures.Size();
	*stream << &numProperties;

	for (SizeType i = 0; i < properties.Size(); i++)
	{
		auto& p = properties[i];

		*stream << p.name;
		*stream << p.displayName;
		*stream << p.description;
		*stream << &p.type;
		*stream << p.UiGroup;
		*stream << &p.uiType;
		*stream << &p.offset;
		*stream << p.initValue;
	}
	for (auto& t : textures)
	{
		FShaderProperty::EType pType = FShaderProperty::TEXTURE_2D;
		FShaderProperty::UiType pUiType = FShaderProperty::BOX;
		FString pUiGroup;

		SizeType reg = t.registerId;

		*stream << t.name;
		*stream << t.displayName;
		*stream << FString();
		*stream << &pType;
		*stream << t.UiGroup;
		*stream << &pUiType;
		*stream << &reg;
		*stream << t.initValue;
	}
}

void CShaderSource::OnDelete()
{
	BaseClass::OnDelete();
	if (auto it = _shaders.Find(this); it != _shaders.end())
		_shaders.Erase(it);
}

bool CShaderSource::Compile()
{
	FShaderSourceFile shader;
	bool result = ParseShaderSourceFile(file->GetSdkPath(), shader);
	THORIUM_ASSERT(result, FString("Failed to parse shader file '") + ToFString(file->Path()) + "'!");
	
	bHasVS = 0;
	bHasPS = 0;
	bHasGEO = 0;

	CFStream stream;

	shaders.Clear();
	
	for (auto& sh : shader.shaders)
	{
		void* data;
		SizeType size;

		if (sh.Key & ShaderType_Vertex)
			gGHI->CompileShader(shader.global + sh.Value, IShader::VFX_VS, &data, &size);
		if (sh.Key & ShaderType_Fragment)
			gGHI->CompileShader(shader.global + sh.Value, IShader::VFX_PS, &data, &size);
		if (sh.Key & ShaderType_Geometry)
			gGHI->CompileShader(shader.global + sh.Value, IShader::VFX_GEO, &data, &size);

		THORIUM_ASSERT(data, "Failed to compile shader!");

		if (data)
		{
			FString shaderName = GetShaderName((EShaderType)sh.Key);
			FString p = file->Dir()->GetPath() + "/vfx/" + file->Name() + ".thcs." + shaderName;
			file->Mod()->CreateFile(p);
			stream.Open(ToFString(file->Mod()->Path() + "/" + p), "wb");
			if (stream.IsOpen())
				stream.Write(data, size);
			stream.Close();

			shaders.Add({ sh.Key, nullptr });
		}
	}

	/*if (!shader.vertexShader.IsEmpty())
	{
		void* vsData;
		SizeType size;
		gRenderer->CompileShader(shader.global + shader.vertexShader, IShader::VFX_VS, &vsData, &size);

		if (!vsData)
			return false;

		bHasVS = 1;

		FString p = file->Dir()->GetPath() + "/vfx/" + file->Name() + ".thcs.vs";
		file->Mod()->CreateFile(p);
		stream.Open(ToFString(file->Mod()->Path() + "/" + p), "wb");
		if (stream.IsOpen())
			stream.Write(vsData, size);
		stream.Close();
	}
	if (!shader.pixelShader.IsEmpty())
	{
		void* vsData;
		SizeType size;
		gRenderer->CompileShader(shader.global + shader.pixelShader, IShader::VFX_PS, &vsData, &size);

		if (!vsData)
			return false;

		bHasPS = 1;

		FString p = file->Dir()->GetPath() + "/vfx/" + file->Name() + ".thcs.ps";
		file->Mod()->CreateFile(p);
		stream.Open(ToFString(file->Mod()->Path() + "/" + p), "wb");
		if (stream.IsOpen())
			stream.Write(vsData, size);
		stream.Close();
	}
	if (!shader.geoShader.IsEmpty())
	{
		void* vsData;
		SizeType size;
		gRenderer->CompileShader(shader.global + shader.geoShader, IShader::VFX_GEO, &vsData, &size);

		if (!vsData)
			return false;

		bHasGEO = 1;

		FString p = file->Dir()->GetPath() + "/vfx/" + file->Name() + ".thcs.gs";
		file->Mod()->CreateFile(p);
		stream.Open(ToFString(file->Mod()->Path() + "/" + p), "wb");
		if (stream.IsOpen())
			stream.Write(vsData, size);
		stream.Close();
	}*/

	shaderName = shader.name;
	description = shader.description;
	type = shader.type;
	bufferSize = shader.bufferSize;

	properties.Clear();
	textures.Clear();

	properties.Add({ "vColorTint", "Color Tint", "", "Color", "{ 1, 1, 1, 1 }", FShaderProperty::VEC4, FShaderProperty::COLOR, 0 });
	properties.Add({ "vNormalIntensity", "Normal Map Strength", "", "Normal", FString(), FShaderProperty::FLOAT, FShaderProperty::SLIDER, 16 });
	properties.Add({ "vAlpha", "Alpha", "", "Color", "1.0", FShaderProperty::FLOAT, FShaderProperty::SLIDER, 20 });

	textures.Add({ "vBaseColor", "Base Color", 5, "Color", "Color(1, 1, 1, 1)"});
	textures.Add({ "vNormalMap", "Normal Map", 6, "Normal", "misc/normal_flat.thtex" });

	for (auto& p : shader.properties)
	{
		if (p.type > FShaderProperty::VEC4)
			textures.Add({ p.internalName, p.name, (uint8)p.bufferOffset, p.initValue });
		else
			properties.Add({ p.internalName, p.name, p.description, p.uiGroup, p.initValue, (FShaderProperty::EType)p.type, (FShaderProperty::UiType)p.uiType, p.bufferOffset });
	}

	Save();

	CONSOLE_LogInfo("CShaderSource", FString("Compiled shader '") + ToFString(file->Name()) + "'!");
	bCompiled = true;

	//delete vsShader;
	//delete psShader;
	//delete geoShader;

	//vsShader = nullptr;
	//psShader = nullptr;
	//geoShader = nullptr;

	LoadShaderObjects();
	return true;
}

void CShaderSource::LoadShaderObjects()
{
	if (!gGHI || !bCompiled)
		return;

	//if (bHasVS && !vsShader)
	//	gRenderer->GetVsShader(this);
	//if (bHasPS && !psShader)
	//	gRenderer->GetPsShader(this);

	for (auto& sh : shaders)
	{
		FString shaderName = GetShaderName((EShaderType)sh.Key);
		FString p = file->Mod()->Path() + "/"  + file->Dir()->GetPath() + "/vfx/" + file->Name() + ".thcs." + shaderName;
		sh.Value = gGHI->LoadShader(this, (EShaderType)sh.Key, p);
	}
}

CShaderSource* CShaderSource::GetShaderSource(const FString& name)
{
	for (auto& it : _shaders)
		if (it->shaderName == name)
			return it;

	return nullptr;
}

IShader* CShaderSource::GetShader(EShaderType_ in)
{
	int pass = (int)in & (ShaderType_DeferredPass | ShaderType_ForwardPass);
	int type = (int)in & (ShaderType_Fragment | ShaderType_Vertex | ShaderType_Geometry);

	for (auto& sh : shaders)
	{
		if ((sh.Key & type) != 0 && ((pass != 0 && (sh.Key & pass)) || pass == 0))
			return sh.Value;
	}
	return nullptr;
}

FString CShaderSource::GetShaderName(EShaderType_ type)
{
	FString shaderName;

	if (type & ShaderType_Vertex)
		shaderName = "vs";
	if (type & ShaderType_Fragment)
		shaderName = "ps";
	if (type & ShaderType_Geometry)
		shaderName = "geo";

	if ((type & ShaderType_DeferredPass) && (type & ShaderType_ForwardPass) == 0)
		shaderName += ".df";
	if ((type & ShaderType_DeferredPass) == 0 && (type & ShaderType_ForwardPass))
		shaderName += ".fwd";

	return shaderName;
}

const TArray<TObjectPtr<CShaderSource>>& CShaderSource::GetAllShaders()
{
	return _shaders;
}

void CShaderSource::LoadVersion05(IBaseFStream* stream)
{
	*stream >> shaderName;
	*stream >> description;
	*stream >> &type;
	*stream >> &bufferSize;

	*stream >> &bHasVS >> &bHasPS >> &bHasGEO;

	uint32 numProperties = 0;
	*stream >> &numProperties;

	for (uint32 i = 0; i < numProperties; i++)
	{
		FString pName;
		FString pDescription;
		int pType;
		FString pUiGroup;
		int pUiType;
		SizeType pBufferOffset;

		*stream >> pName;
		*stream >> pDescription;
		*stream >> &pType;
		*stream >> pUiGroup;
		*stream >> &pUiType;
		*stream >> &pBufferOffset;

		if (pType > FShaderProperty::VEC4)
		{
			FShaderTexture tex{ pName, pName, (uint8)pBufferOffset, FString(), pUiGroup};
			textures.Add(tex);
		}
		else
		{
			FShaderProperty p{ pName, pName, pDescription, pUiGroup, FString(), (FShaderProperty::EType)pType, (FShaderProperty::UiType)pUiType, pBufferOffset};
			properties.Add(p);
		}
	}

	bInitialized = true;
}

uint8 CShaderSource::GetFileVersion() const
{
	return THCS_VERSION;
}
