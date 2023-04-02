
#include "ShaderParser.h"
#include "Renderer.h"
#include "Console.h"

#include <Util/FStream.h>

#define THCS_VERSION 6

#define THCS_MAGIC_SIZE 28
static const char* thcsMagicStr = "\0\0ThoriumEngine Shader File\0";

static TArray<TObjectPtr<CShaderSource>> _shaders;

static CConCmd cmdDebugShader("shader.printinfo", [](const TArray<FString>& args) {
	if (args.Size() == 0)
		return;
	
	CShaderSource* shader = CShaderSource::GetShader(args[0]);
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
	delete vsShader;
	delete psShader;
	delete geoShader;
}

void CShaderSource::Init()
{
	// Add the static properties and textures.
	//properties.Add({ "vColorTint", "Color Tint", "", "Color", FShaderProperty::VEC4, FShaderProperty::COLOR, 0});
	//properties.Add({ "vNormalIntensity", "Normal Map Strength", "", "Normal", FShaderProperty::FLOAT, FShaderProperty::SLIDER, 16 });
	//properties.Add({ "vAlpha", "Alpha", "", "Color", FShaderProperty::FLOAT, FShaderProperty::SLIDER, 20});

	//textures.Add({ "vBaseColor", "Base Color", 5, "Color" });
	//textures.Add({ "vNormalMap", "Normal Map", 6, "Normal" });

	// Register the shader
	_shaders.Add(this);

	TUniquePtr<IBaseFStream> stream = file->GetStream("rb");
	if (!stream->IsOpen())
	{
		CONSOLE_LogError("CShaderSource", FString("Failed to create filestream for Shader '") + ToFString(file->Path()) + "'");
		return;
	}

	char magicStr[THCS_MAGIC_SIZE];
	stream->Read(magicStr, THCS_MAGIC_SIZE);
	if (memcmp(thcsMagicStr, magicStr, THCS_MAGIC_SIZE) != 0)
	{
		// Check if this file isn't empty, if it is, try to compile it.
		if (stream->Tell() == 0)
		{
			stream = nullptr;
			if (Compile())
				bInitialized = true;
		}
		else
			CONSOLE_LogError("CShaderSource", FString("Invalid Shader file '") + ToFString(file->Path()) + "'");
		return;
	}

	bCompiled = true;

	uint16 version;
	*stream >> &version;
	
	if (version != THCS_VERSION)
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

	*stream >> &bHasVS >> &bHasPS >> &bHasGEO;

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

		*stream >> pName;
		*stream >> pDisplayName;
		*stream >> pDescription;
		*stream >> &pType;
		*stream >> pUiGroup;
		*stream >> &pUiType;
		*stream >> &pBufferOffset;

		if (pType > FShaderProperty::VEC4)
		{
			FShaderTexture tex{ pName, pDisplayName, (uint8)pBufferOffset, pUiGroup };
			textures.Add(tex);
		}
		else
		{
			FShaderProperty p{ pName, pDisplayName, pDescription, pUiGroup, (FShaderProperty::EType)pType, (FShaderProperty::UiType)pUiType, pBufferOffset };
			properties.Add(p);
		}
	}

	bInitialized = true;
}

void CShaderSource::Save()
{
	TUniquePtr<IBaseFStream> stream = file->GetStream("wb");
	if (!stream->IsOpen())
	{
		CONSOLE_LogError("CShaderSource", FString("Failed to create file stream for '") + ToFString(file->Path()) + "'");
		return;
	}

	stream->Write((void*)thcsMagicStr, THCS_MAGIC_SIZE);

	uint16 version = THCS_VERSION;
	*stream << &version;

	*stream << shaderName;
	*stream << description;
	*stream << (int8*)&type;
	*stream << &bufferSize;

	*stream << &bHasVS << &bHasPS << &bHasGEO;

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
	}
}

void CShaderSource::OnDelete()
{
	BaseClass::OnDelete();
	_shaders.Erase(_shaders.Find(this));
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
	
	if (!shader.vertexShader.IsEmpty())
	{
		void* vsData;
		SizeType size;
		gRenderer->CompileShader(shader.global + shader.vertexShader, IShader::VFX_VS, &vsData, &size);

		if (!vsData)
			return false;

		bHasVS = 1;

		WString p = file->Dir()->GetPath() + L"\\vfx\\" + file->Name() + L".thcs.vs";
		file->Mod()->CreateFile(p);
		stream.Open(ToFString(file->Mod()->Path() + L"\\" + p), "wb");
		if (stream.IsOpen())
			stream.Write(vsData, size);
	}
	if (!shader.pixelShader.IsEmpty())
	{
		void* vsData;
		SizeType size;
		gRenderer->CompileShader(shader.global + shader.pixelShader, IShader::VFX_PS, &vsData, &size);

		if (!vsData)
			return false;

		bHasPS = 1;

		WString p = file->Dir()->GetPath() + L"\\vfx\\" + file->Name() + L".thcs.ps";
		file->Mod()->CreateFile(p);
		stream.Open(ToFString(file->Mod()->Path() + L"\\" + p), "wb");
		if (stream.IsOpen())
			stream.Write(vsData, size);
	}
	if (!shader.geoShader.IsEmpty())
	{
		void* vsData;
		SizeType size;
		gRenderer->CompileShader(shader.global + shader.geoShader, IShader::VFX_GEO, &vsData, &size);

		if (!vsData)
			return false;

		bHasGEO = 1;

		WString p = file->Dir()->GetPath() + L"\\vfx\\" + file->Name() + L".thcs.gs";
		file->Mod()->CreateFile(p);
		stream.Open(ToFString(file->Mod()->Path() + L"\\" + p), "wb");
		if (stream.IsOpen())
			stream.Write(vsData, size);
	}
	stream.Close();

	shaderName = shader.name;
	description = shader.description;
	type = shader.type;
	bufferSize = shader.bufferSize;

	properties.Clear();
	textures.Clear();

	properties.Add({ "vColorTint", "Color Tint", "", "Color", FShaderProperty::VEC4, FShaderProperty::COLOR, 0 });
	properties.Add({ "vNormalIntensity", "Normal Map Strength", "", "Normal", FShaderProperty::FLOAT, FShaderProperty::SLIDER, 16 });
	properties.Add({ "vAlpha", "Alpha", "", "Color", FShaderProperty::FLOAT, FShaderProperty::SLIDER, 20 });

	textures.Add({ "vBaseColor", "Base Color", 5, "Color" });
	textures.Add({ "vNormalMap", "Normal Map", 6, "Normal" });

	for (auto& p : shader.properties)
	{
		if (p.type > FShaderProperty::VEC4)
			textures.Add({ p.internalName, p.name, (uint8)p.bufferOffset });
		else
			properties.Add({ p.internalName, p.name, p.description, p.uiGroup, (FShaderProperty::EType)p.type, (FShaderProperty::UiType)p.uiType, p.bufferOffset });
	}

	Save();

	CONSOLE_LogInfo("CShaderSource", FString("Compiled shader '") + ToFString(file->Name()) + "'!");
	bCompiled = true;

	delete vsShader;
	delete psShader;

	vsShader = nullptr;
	psShader = nullptr;

	LoadShaderObjects();
	return true;
}

void CShaderSource::LoadShaderObjects()
{
	if (!gRenderer || !bCompiled)
		return;

	if (bHasVS && !vsShader)
		gRenderer->GetVsShader(this);
	if (bHasPS && !psShader)
		gRenderer->GetPsShader(this);
}

CShaderSource* CShaderSource::GetShader(const FString& name)
{
	for (auto& it : _shaders)
		if (it->shaderName == name)
			return it;

	return nullptr;
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
			FShaderTexture tex{ pName, pName, (uint8)pBufferOffset, pUiGroup };
			textures.Add(tex);
		}
		else
		{
			FShaderProperty p{ pName, pName, pDescription, pUiGroup, (FShaderProperty::EType)pType, (FShaderProperty::UiType)pUiType, pBufferOffset};
			properties.Add(p);
		}
	}

	bInitialized = true;
}
