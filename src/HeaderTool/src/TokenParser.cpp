
#include <string>
#include <iostream>
#include "TokenParser.h"
#include "CppTypes.h"

enum EReadMode
{
	RM_HEADER_BASE,
	RM_CLASS,
	RM_ENUM,
	RM_PROPERTY,
	RM_FUNCTION,
};

enum EClassReadStage
{
	CRS_ClassDef,
	CRS_ClassGeneratedMacro,
	CRS_ClassFields
};

static const char* _MacroNames[] = {
	"STRUCT",
	"CLASS",
	"ENUM",
	"PROPERTY",
	"FUNCTION",
	"GENERATED_BODY",
	"ASSET",
	"META"
};

static const char* SupportedTemplates[] = {
	"TObjectPtr",
	"TArray",
	"TEnum",
	"TAssetRef",
	"TClassPtr"
};
constexpr SizeType numSupportedTemplate = sizeof(SupportedTemplates) / sizeof(const char*);

static const char* SupportedSubTemplatable[] = {
	"TArray",
};
constexpr SizeType numSupportedSubTemplatable = sizeof(SupportedSubTemplatable) / sizeof(const char*);

static bool IsTemplateSupported(const FString& t)
{
	for (SizeType i = 0; i < numSupportedTemplate; i++)
	{
		if (t == SupportedTemplates[i])
			return true;
	}

	return false;
}

static bool TemplateSupportsSubTemplate(const FString& t)
{
	for (SizeType i = 0; i < numSupportedSubTemplatable; i++)
	{
		if (t == SupportedSubTemplatable[i])
			return true;
	}
	return false;
}

inline EReadMode MacroTypeToReadMode(FMacro::EType type)
{
	return (type == FMacro::CLASS || type == FMacro::STRUCT || type == FMacro::ASSET) ? RM_CLASS : (type == FMacro::ENUM) ? RM_ENUM : RM_HEADER_BASE;
}

static int TryReadMacro(std::vector<FToken>::iterator it, FMacro& out)
{
	out = FMacro();

	int i;
	for (i = 0; i < FMacro::ETYPE_END; i++)
	{
		if (it[0].text == _MacroNames[i])
			break;
	}

	if (i == FMacro::ETYPE_END)
		return 0;

	out.type = (FMacro::EType)i;
	out.line = it[0].line;

	i = 1;
	if (it[i].type != FToken::ParenthesisOpen)
	{
		//std::cerr << "ERROR - Invalid macro at line: " << it[i].line << "\n";
		// probably just a normal symbol with the same name as a macro. so we just ignore it instead of treating it as an error.
		return 0;
	}

	++i;
	while (it[i].type != FToken::ParenthesisClose)
	{
		if (it[i].type == FToken::Seperator)
			++i;

		if (it[i].type == FToken::Symbol || it[i].type == FToken::StringLiteral)
		{
			FString key = it[i].text;
			FString value;
			
			++i;
			if (it[i].type == FToken::Operator && it[i].text == "=")
			{
				++i;
				if (it[i].type == FToken::Symbol || it[i].type == FToken::StringLiteral || it[i].type == FToken::NumericLiteral)
				{
					value = it[i].text;
					++i;
				}
				else
				{
					std::cerr << "WARNING - Invalid macro argument value at line: " << it[i].line << "\n";
					return 0;
				}

				out.Arguments.Add({ key, value });
			}
			else if (it[i].type == FToken::Seperator || it[i].type == FToken::ParenthesisClose)
				out.Arguments.Add({ key, value });
		}
		else
		{
			std::cerr << "WARNING - Invalid macro argument at line: " << it[i].line << "\n";
			return 0;
		}
	}
	return i;
}

int TryReadProperty(std::vector<FToken>::iterator it, CppProperty& out, bool bIsTemplate = false)
{
	int i = 0;
	for (;;i++)
	{
		if (!bIsTemplate)
		{
			if (it[i].type == FToken::Seperator || it[i].type == FToken::ParenthesisClose)
			{
				if (out.typeName.IsEmpty())
					out.typeName = it[i - 2].text;

				out.name = it[i - 1].text;
				break;
			}
		}
		else
		{
			if (it[i].type == FToken::Seperator)
			{
				if (out.typeName.IsEmpty())
					out.typeName = it[i - 1].text;
				break;
			}
		}

		if (it[i].type == FToken::ParenthesisOpen)
		{
			// This must be a function. so we stop reading the property here and let the function reader handle it.
			if (out.typeName.IsEmpty())
				out.typeName = it[i - 2].text;
			break;
		}

		if (it[i].type == FToken::Keyword)
		{
			if (it[i].text == "static")
			{
				out.bStatic = true;
				continue;
			}
			else if (it[i].text == "const")
			{
				out.bConst = true;
				continue;
			}
		}

		if (it[i].type == FToken::Operator)
		{
			if (it[i].text == "<")
			{
				out.typeName = it[i - 1].text;
				out.bTemplateType = true;
				do {
					CppProperty templateArg;
					if (int j = TryReadProperty(it + i + 1, templateArg, true); j > 0)
					{
						out.templateArgs.Add(templateArg);
						i += j + 1;
						continue;
					}
					else
					{
						std::cerr << "ERROR - Invalid template argument at line: " << it[i].line << "\n";
						return -1;
					}
				}
				while (it[i].type == FToken::Seperator);
				continue;
			}
			if (it[i].text == ">")
			{
				if (bIsTemplate)
				{
					if (out.typeName.IsEmpty())
						out.typeName = it[i - 1].text;
					else if (out.name.IsEmpty())
						out.name = it[i - 1].text;
					break;
				}
				continue;
			}

			if (it[i].text == "*")
			{
				out.bPointer = true;
				if (out.typeName.IsEmpty())
					out.typeName = it[i - 1].text;
				continue;
			}
			if (it[i].text == "&")
			{
				out.bRef = true;
				if (out.typeName.IsEmpty())
					out.typeName = it[i - 1].text;
				continue;
			}
		}

		if (it[i].text == "=" || it[i].type == FToken::EndOfInstruction || it[i].text == ":")
		{
			if (out.typeName.IsEmpty())
				out.typeName = it[i - 2].text;

			out.name = it[i - 1].text;
			break;
		}
	}

	return i;
}

int CTokenParser::ParseHeader(FHeaderData& data)
{
	CTokenizer tokenizer;
	std::vector<FToken> tokens;
	if (!tokenizer.ParseFile(data.FilePath.c_str(), tokens))
	{
		std::cout << "Failed to parse file: " << data.FilePath.c_str() << std::endl;
		return 1;
	}

	uint generatedIncludeLine = -1;
	EReadMode rm = RM_HEADER_BASE;
	int indent = 0;
	int readStage = 0;
	TArray<FString> ifStack;

	bool bPublic = false;

	bool bInMacro = false;
	FMacro readMacro;

	for (auto it = tokens.begin(); it < tokens.end(); it++)
	{
		/*if (it->type == FToken::ScopeOpen)
			indent++;
		else if (it->type == FToken::ScopeClose)
			indent--;*/
		
		if (rm == RM_HEADER_BASE)
		{
			if (it->type == FToken::Symbol && it->text == "#include")
			{
				if (it[1].text.find(".generated.h") != -1)
				{
					generatedIncludeLine = it[1].line;
					++it;
					continue;
				}
				else
				{
					if (generatedIncludeLine < it[0].line)
					{
						std::cerr << "ERROR - generated head must be the last include in the file! line: " << it[0].line << std::endl;
						return 2;
					}
				}
			}
		}

		// Ignore macro definitions or anything that starts with #
		{
			if (bInMacro)
			{
				int line = it[0].line;
				while (it != tokens.end() && it[0].line == line)
					it++;

				it--;
				if (it[0].type == FToken::Operator && it[0].text == "\\")
					continue;

				bInMacro = false;
				continue;
			}

			if (it[0].type == FToken::Symbol && it[0].text[0] == '#')
			{
				if (it[0].text == "#if")
				{
					ifStack.Add();

					auto _it = it + 1;
					while (_it->line == it[0].line)
					{
						(*ifStack.last()) += _it->text;
						_it++;
					}
					continue;
				}
				if (it[0].text == "#endif" && ifStack.Size() > 0)
				{
					ifStack.PopBack();
					continue;
				}
				int line = it[0].line;
				while (it != tokens.end() && it[0].line == line)
					it++;

				it--; // go back to the last token of this line.
				if (it[0].type == FToken::Operator && it[0].text == "\\")
					bInMacro = true;
				continue;
			}

		}

		if (rm == RM_HEADER_BASE)
		{
			// Read until we find a macro.
			int i = TryReadMacro(it, readMacro);
			if (i > 0)
			{
				rm = MacroTypeToReadMode(readMacro.type);
				it += i;
				readStage = 0;
				continue;
			}
		}

		if (rm == RM_CLASS)
		{
			if (readStage == CRS_ClassDef)
			{
				if (it[0].type == FToken::Keyword && (it[0].text == "class" || it[0].text == "struct"))
				{
					data.classes.Add();
					CppClass& _class = *data.classes.last();
					_class.classMacro = readMacro;
					if (ifStack.Size() > 0)
						_class.IfGuard = *ifStack.last();
					it++;
					for (;;it++)
					{
						if (it[0].type == FToken::Operator && it[0].text == ":")
						{
							_class.name = it[-1].text;
							it++;
							if (it[0].type == FToken::Keyword && (it[0].text == "public" || it[0].text == "private" || it[0].text == "protected"))
							{
								it++;
								_class.baseName = it[0].text;
							}
							else
								_class.baseName = it[0].text;

							if (it[1].type == FToken::Seperator)
							{
								std::cerr << "ERROR - Multiple inheritance is not supported! line: " << it[0].line << "\n";
								return 8;
							}
						}
						else if (it[0].type == FToken::ScopeOpen)
						{
							if (_class.name.IsEmpty())
								_class.name = it[-1].text;
							break;
						}
					}

					readStage = CRS_ClassGeneratedMacro;
					continue;
				}
			}
			if (readStage == CRS_ClassGeneratedMacro)
			{
				FMacro genMacro;
				if (int i = TryReadMacro(it, genMacro); i > 0)
				{
					if (genMacro.type != FMacro::GENERATED_BODY)
					{
						std::cerr << "ERROR - Expected GENERATED_BODY macro after class definition at line: " << it[0].line << "\n";
						return 4;
					}
					data.classes.last()->bodyMacro = genMacro;
					readStage = CRS_ClassFields;
					it += i;
				}
				continue;
			}
			if (readStage == CRS_ClassFields)
			{
				CppClass& _class = *data.classes.last();

				// embedded class/struct/enum definitions are not supported.
				// but we want to parse them anyway to avoid parsing errors in the rest of the file.
				if (it[0].type == FToken::ScopeOpen)
				{
					int indent = 1;

					while (indent > 0)
					{
						it++;
						if (it->type == FToken::ScopeOpen)
							indent++;
						else if (it->type == FToken::ScopeClose)
							indent--;
					}
					continue;
				}

				if (it[0].type == FToken::ScopeClose && it[1].type == FToken::EndOfInstruction)
				{
					rm = RM_HEADER_BASE;
					continue;
				}

				if (it[0].type == FToken::Keyword && it[0].text == "public" && it[1].text == ":")
				{
					bPublic = true;
					it++;
					continue;
				}
				if (it[0].type == FToken::Keyword && it[0].text == "private" && it[1].text == ":")
				{
					bPublic = false;
					it++;
					continue;
				}
				if (it[0].type == FToken::Keyword && it[0].text == "protected" && it[1].text == ":")
				{
					bPublic = false;
					it++;
					continue;
				}

				if (int i = TryReadMacro(it, readMacro); i > 0)
				{
					if (readMacro.type == FMacro::PROPERTY)
					{
						_class.Properties.Add();
						if (ifStack.Size() > 0)
							(*_class.Properties.last()).IfGuard = *ifStack.last();

						CppProperty& property = *_class.Properties.last();
						property.macro = readMacro;
						property.line = it[0].line;
						property.bPrivate = !bPublic;
						rm = RM_PROPERTY;
					}
					if (readMacro.type == FMacro::FUNCTION)
					{
						_class.Functions.Add();
						if (ifStack.Size() > 0)
							(*_class.Functions.last()).IfGuard = *ifStack.last();

						CppFunction& func = *_class.Functions.last();
						func.macro = readMacro;
						func.line = it[0].line;
						func.bPrivate = !bPublic;
						rm = RM_FUNCTION;
					}
					it += i;
				}
				continue;
			}
			continue;
		}
		if (rm == RM_PROPERTY)
		{
			CppClass& _class = *data.classes.last();
			CppProperty& property = *_class.Properties.last();

			if (int i = TryReadProperty(it, property); i > 0)
			{
				rm = RM_CLASS;
				it += i;
				while (it[0].type != FToken::EndOfInstruction)
					it++;
				continue;
			}

			/*if (it[0].type == FToken::Keyword)
			{
				if (it[0].text == "const")
					property.bConst = true;
				else if (it[0].text == "static")
					property.bStatic = true;

				it++;
			}

			if (it[0].type == FToken::Operator)
			{
				if (it[0].text == "*")
					property.bPointer = true;
				else if (it[0].text == "&")
					property.bRef = true;
			}

			if (it[0].type == FToken::Operator && it[0].text == "<")
			{
				property.templateTypename = it[-1].text;
				property.bTemplateType = true;

				if (!IsTemplateSupported(property.templateTypename))
				{
					std::cerr << "ERROR - Unsupported template '" << property.typeName.c_str() << "'! line:" << it[0].line << "\n";
					return 3;
				}
				continue;
			}
			if (it[1].type == FToken::Operator)
			{
				if (it[1].text == ">" && property.bTemplateType)
				{
					property.typeName = it[0].text;
				}
				if (it[1].text == "<" && property.bTemplateType)
				{
					if (!TemplateSupportsSubTemplate(property.templateTypename))
					{
						std::cerr << "ERROR - Template type " << property.templateTypename.c_str() << " does not support nested templates! line: " << it[0].line << "\n";
						return 4;
					}

					property.nestedTemplateType = it[0].text;
					it += 2;
					property.typeName = it[0].text;
					it++;
					continue;
				}
			}

			if (it[0].type == FToken::EndOfInstruction || (it[0].type == FToken::Operator && (it[0].text == "=" || it[0].text == ":")))
			{
				property.name = it[-1].text;

				uint8 templateCount = 0;
				if (!property.templateTypename.IsEmpty())
				{
					property.fullTypename += property.templateTypename + "<";
					templateCount++;
				}
				if (!property.nestedTemplateType.IsEmpty())
				{
					property.fullTypename += property.nestedTemplateType + "<";
					templateCount++;
				}

				property.fullTypename += property.typeName;

				for (uint8 i = 0; i < templateCount; i++)
					property.fullTypename += ">";

				rm = RM_CLASS;
			}
			continue;*/
		}
		if (rm == RM_FUNCTION)
		{
			CppClass& _class = *data.classes.last();
			CppFunction& func = *_class.Functions.last();

			if (it[0].type == FToken::Keyword)
			{
				if (it[0].text == "static")
				{
					func.bStatic = true;
					continue;
				}
				else if (it[0].text == "virtual")
				{
					func.bVirtual = true;
					continue;
				}
				else if (it[0].text == "inline")
				{
					func.bInline = true;
					continue;
				}
			}

			if (func.name.IsEmpty())
			{
				if (int i = TryReadProperty(it, func.returnValue); i > 0)
				{
					it += i;
					func.name = it[-1].text;
				}
			}
			it++; // skip function name and the opening parenthesis.

			while (it[0].type != FToken::ParenthesisClose)
			{
				if (it[0].type == FToken::Seperator)
					it++;

				CppProperty argument{};
				int i = TryReadProperty(it, argument);
				if (i > 0)
				{
					func.Arguments.Add(argument);
					it += i;
				}
				else
					it++;
			}
			it++;

			if (it[0].type == FToken::ScopeOpen)
			{
				int indent = 1;

				while (indent > 0)
				{
					it++;
					if (it->type == FToken::ScopeOpen)
						indent++;
					else if (it->type == FToken::ScopeClose)
						indent--;
				}
			}

			CppFunction::EType type = CppFunction::GENERAL;

			if (func.macro.ArgIndex("ServerRpc") != -1)
				type = CppFunction::SERVER_RPC;
			if (func.macro.ArgIndex("ClientRpc") != -1)
				type = CppFunction::CLIENT_RPC;
			if (func.macro.ArgIndex("MulticastRpc") != -1)
				type = CppFunction::MUTLICAST_RPC;
			if (func.macro.ArgIndex("Output") != -1)
				type = CppFunction::OUTPUT;
			if (func.macro.ArgIndex("ConCmd") != -1)
				type = CppFunction::COMMAND;

			func.type = type;

			rm = RM_CLASS;
			continue;
		}
		if (rm == RM_ENUM)
		{
			data.enums.Add();
			CppEnum& _enum = *data.enums.last();
			if (it[0].type == FToken::Keyword && it[0].text == "enum")
			{
				it++;
				if (it[0].text == "class")
					it++;
				_enum.name = it[0].text;
				it++;
			}

			if (it[0].type == FToken::Operator && it[0].text == ":")
			{
				it++;
				_enum.base = it[0].text;
				it++;
			}

			if (it[0].type != FToken::ScopeOpen)
			{
				std::cerr << "ERROR - Expected '{' after enum definition at line: " << it[0].line << "\n";
				return 5;
			}
			it++;

			while (it[0].type != FToken::ScopeClose)
			{
				if (it[0].type == FToken::Seperator)
				{
					it++;
					continue;
				}
				FEnumVariable var;
				var.name = it[0].text;
				it++;
				if (it[0].type == FToken::Operator && it[0].text == "=")
				{
					it++;
					while (it[0].type != FToken::Seperator && it[0].type != FToken::ScopeClose && it[0].text != "META")
					{
						var.value += it[0].text;
						it++;
					}
				}
				if (it[0].text == "META")
				{
					FMacro meta;
					if (int i = TryReadMacro(it, meta); i > 0)
					{
						var.meta = meta;
						it += i + 1;
					}
					else
						std::cerr << "WARNING - Invalid META macro for enum variable at line: " << it[0].line << "\n";
				}
				_enum.Values.Add(var);
			}
			it++;

			rm = RM_HEADER_BASE;
		}
	}

	data.bEmpty = data.classes.Size() == 0 && data.enums.Size() == 0;
	return 0;
}
