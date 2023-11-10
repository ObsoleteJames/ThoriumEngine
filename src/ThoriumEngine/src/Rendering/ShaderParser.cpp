
#include <string>
#include "ShaderParser.h"
#include "Shader.h"
#include <fstream>

enum EShaderBlock
{
	EShaderBlock_NONE,
	EShaderBlock_SHADER,
	EShaderBlock_GLOBAL,
	EShaderBlock_VERTEX,
	EShaderBlock_PIXEL,
	EShaderBlock_GEO
};

bool ParseShaderSourceFile(const WString& file, FShaderSourceFile& out)
{
	std::ifstream stream(file.c_str());
	if (!stream.is_open())
		return false;

	out.file = file;

	FString value;
	FString prevValue;
	EShaderBlock curBlock = EShaderBlock_NONE;
	bool bInQuotes = false;
	bool bPrevSpace = false;
	int blockIndex = 0;
	int curMode = 0;
	
	std::string _l;
	while (std::getline(stream, _l))
	{
		if (SizeType i = _l.find("//"); i != -1)
			_l.erase(_l.begin() + i, _l.end());

		prevValue = value;
		value.Clear();
		bPrevSpace = true;
		curMode = 0;

		for (char ch : _l)
		{
			if (!bInQuotes && (ch == ' ' || ch == '\t'))
			{
				if (!bPrevSpace)
				{
					prevValue = value;
					value.Clear();
				}
				bPrevSpace = true;
				continue;
			}

			if (ch == '"')
			{
				bInQuotes = !bInQuotes;
				bPrevSpace = false;
				continue;
			}

			if (ch == '{' && curMode != 3)
				blockIndex++;

			if (ch == '}' && curMode != 3)
				blockIndex--;

			if (curBlock == EShaderBlock_NONE && ch == '{')
			{
				if (prevValue == "Shader")
				{
					curBlock = EShaderBlock_SHADER;
					curMode = 0;
					value.Clear();
					continue;
				}
				if (prevValue == "Global")
				{
					curBlock = EShaderBlock_GLOBAL;
					curMode = 0;
					value.Clear();
					continue;
				}
				if (prevValue == "VS")
				{
					curBlock = EShaderBlock_VERTEX;
					curMode = 0;
					value.Clear();
					continue;
				}
				if (prevValue == "PS")
				{
					curBlock = EShaderBlock_PIXEL;
					curMode = 0;
					value.Clear();
					continue;
				}
				if (prevValue == "GEO")
				{
					curBlock = EShaderBlock_GEO;
					curMode = 0;
					value.Clear();
					continue;
				}
			}

			if (blockIndex == 0)
				curBlock = EShaderBlock_NONE;

			if (curBlock == EShaderBlock_SHADER)
			{
				if (ch == '=')
					continue;

				if (ch == ';')
				{
					if (prevValue == "Type")
					{
						if (value == "SHADER_INTERNAL")
							out.type = CShaderSource::ST_INTERNAL;
						else if (value == "SHADER_FORWARD")
							out.type = CShaderSource::ST_FORWARD;
						else if (value == "SHADER_DEFERRED")
							out.type = CShaderSource::ST_DEFERRED;
						else if (value == "SHADER_POSTPROCESSING")
							out.type = CShaderSource::ST_POSTPROCESS;
						else if (value == "SHADER_DEBUG")
							out.type = CShaderSource::ST_DEBUG;
						else
							out.type = CShaderSource::ST_UNKNOWN;
					}
					else if (prevValue == "Name")
						out.name = value;
				}
			}
			if (curBlock == EShaderBlock_GLOBAL)
			{
				if (ch == '<' && value == "Property")
				{
					curMode = 1;
					out.properties.Add();
				}
				
				if (curMode == 1 && ch == '>')
				{
					FString type = value;
					type.Erase(type.begin(), type.begin() + 9);

					int iType = 0;

					if (type == "int")
						iType = FShaderProperty::INT;
					else if (type == "float")
						iType = FShaderProperty::FLOAT;
					else if (type == "bool")
						iType = FShaderProperty::BOOL;
					else if (type == "float3")
						iType = FShaderProperty::VEC3;
					else if (type == "float4")
						iType = FShaderProperty::VEC4;
					else if (type == "Texture2D")
						iType = FShaderProperty::TEXTURE_2D;
					else if (type == "TextureCube")
						iType = FShaderProperty::TEXTURE_CUBE;

					out.properties.last()->type = iType;
					curMode = 2;
				}

				if (curMode == 2)
				{
					if (ch == '(')
					{
						prevValue = value;
						value.Clear();
						bPrevSpace = true;
						out.properties.last()->internalName = prevValue;
						continue;
					}

					if (ch == '=')
						continue;

					if (ch == ',' || ch == ')')
					{
						if (prevValue == "UiType")
						{
							int uiType = 0;
							if (value == "SLIDER")
								uiType = 1;
							else if (value == "COLOR")
								uiType = 2;

							out.properties.last()->uiType = uiType;
						}
						else if (prevValue == "Description")
							out.properties.last()->description = value;
						else if (prevValue == "Name")
							out.properties.last()->name = value;
						else if (prevValue == "Group")
							out.properties.last()->uiGroup = value;

						if (ch == ')')
							curMode = 3;

						continue;
					}

					if (curMode == 3)
					{
						if (ch == ';')
						{
							out.properties.last()->initValue = value;
							curMode = 2;
						}
					}
				}
			}

			value += ch;
			bPrevSpace = false;
		}

		if (curBlock == EShaderBlock_VERTEX)
		{
			out.vertexShader += _l + "\n";
		}
		if (curBlock == EShaderBlock_PIXEL)
		{
			out.pixelShader += _l + "\n";
		}
		if (curBlock == EShaderBlock_GEO)
		{
			out.geoShader += _l + "\n";
		}

		if (curBlock == EShaderBlock_GLOBAL && curMode == 0)
			out.global += _l + "\n";
	}

	if (!out.global.IsEmpty())
		out.global.Erase(out.global.begin());
	if (!out.vertexShader.IsEmpty())
		out.vertexShader.Erase(out.vertexShader.begin());
	if (!out.pixelShader.IsEmpty())
		out.pixelShader.Erase(out.pixelShader.begin());
	if (!out.geoShader.IsEmpty())
		out.geoShader.Erase(out.geoShader.begin());

	FString textureCode;
	FString shaderBuffer = 
		"\ncbuffer ShaderBuffer : register(b6) \n{\n"
		"\tfloat4 vColorTint;\n"
		"\tfloat vNormalIntensity;\n"
		"\tfloat vAlpha;\n"
		"\tfloat2 _padding_vAlpha;";

	std::vector<FShaderSourceFile::FProperty*> properties;
	for (auto& p : out.properties)
		properties.push_back(&p);

	std::sort(properties.begin(), properties.end(), 
		[](FShaderSourceFile::FProperty* a, FShaderSourceFile::FProperty* b) { return a->type > b->type; });

	int texIndex = 7;
	SizeType bufferOffset = 32;

	constexpr SizeType typeSizes[] = {
		0,
		4,
		4,
		4,
		12,
		16,
	};

	const char* typeNames[] = {
		"ERR",
		"bool",
		"int",
		"float",
		"float3",
		"float4",
	};

	for (auto* p : properties)
	{
		if (p->type > FShaderProperty::VEC4)
		{
			textureCode += (p->type == FShaderProperty::TEXTURE_2D ? FString("Texture2D") : FString("TextureCube")) + " " + p->internalName + " : TEXTURE : register(t" + FString::ToString(texIndex) + ");\n";
			textureCode += FString("SamplerState ") + p->internalName + "Sampler : SAMPLER : register(s" + FString::ToString(texIndex) + ");\n";
			p->bufferOffset = (SizeType)texIndex;
			texIndex++;
			continue;
		}

		p->bufferOffset = bufferOffset;
		shaderBuffer += FString("\t") + typeNames[p->type] + " " + p->internalName + ";\n";
		
		bufferOffset += typeSizes[p->type];

		if (p->type == FShaderProperty::VEC3)
		{
			shaderBuffer += FString("\tfloat _padding_") + p->internalName + ";\n";
			bufferOffset += 4;
		}
	}

	out.bufferSize = bufferOffset;

	out.global += shaderBuffer + "\n}\n\n";
	out.global += textureCode;
	out.global += "\n\n";
	
	return true;
}
