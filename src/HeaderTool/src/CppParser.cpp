
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include "CppParser.h"

struct FComment
{
	FString text;
	int lineEnd;
};

static int CurLine = 0;

static const char* SupportedTemplates[] = {
	"TObjectPtr",
	"TArray",
	//"TMap",
	"TEnum",
	"TAssetRef",
	"TClassPtr"
};
constexpr SizeType numSupportedTemplate = sizeof(SupportedTemplates) / sizeof(const char*);

static const char* SupportedSubTemplatable[] = {
	"TArray",
};
constexpr SizeType numSupportedSubTemplatable = sizeof(SupportedSubTemplatable) / sizeof(const char*);

static const char* _MacroNames[] = {
	"STRUCT",
	"CLASS",
	"ENUM",
	"PROPERTY",
	"FUNCTION",
	"GENERATED_BODY",
	"ASSET",
	"MACRO"
};

static const char* _FunctionKeywords[] = {
	"inline",
	"static",
	"virtual",
};
constexpr SizeType numFunctionKeywords = sizeof(_FunctionKeywords) / sizeof(const char*);

enum EReadMode
{
	RM_HEADER_BASE,
	RM_CLASS,
	RM_ENUM,
	RM_PROPERTY,
	RM_FUNCTION,
};

int readStage = 0;
int bPublic = false;
TArray<FString> IfStack;

inline EReadMode MacroTypeToReadMode(FMacro::EType type)
{
	return (type == FMacro::CLASS || type == FMacro::STRUCT || type == FMacro::ASSET) ? RM_CLASS : (type == FMacro::ENUM) ? RM_ENUM : RM_HEADER_BASE;
}

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

static uint32_t IsFunctionKeyword(const FString& in)
{
	for (SizeType i = 0; i < numFunctionKeywords; i++)
	{
		if (in == _FunctionKeywords[i])
			return (uint32_t)i + 1;
	}
	return 0;
}

static int TryReadMacro(const FString& line, FMacro& out)
{
	out = FMacro();

	SizeType pos = -1;
	int i;
	for (i = 0; i < FMacro::ETYPE_END; i++)
	{
		pos = line.Find(_MacroNames[i]);
		if (pos != FString::npos)
			break;
	}

	if (pos == -1)
		return 0;

	out.type = (FMacro::EType)i;
	out.line = CurLine;

	SizeType bP = line.FindFirstOf('(', pos);
	if (bP == -1)
	{
		//std::cerr << "ERROR - Invalid macro at line: " << CurLine << "\n";
		return 0;
	}

	bool bReadValue = true;
	bool bInQuotes = false;
	bool bLastIsSpace = false;
	FString value;
	FString key;
	const char* read = line.c_str() + bP + 1;
	while (true)
	{
		if (read[0] == '\0')
			break;

		if (read[0] == ' ' && !bInQuotes)
		{
			bLastIsSpace = true;
			read++;
			continue;
		}

		if (read[0] == '"')
		{
			bInQuotes ^= 1;
			bLastIsSpace = false;
			read++;
			continue;
		}

		if (read[0] == ')')
		{
			if (!value.IsEmpty())
				out.Arguments.Add({ value, key });
			break;
		}

		if (read[0] == ',' && !bInQuotes)
		{
			out.Arguments.Add({ value, key });

			bReadValue = true;
			bLastIsSpace = false;
			value.Clear();
			key.Clear();
			read++;
			continue;
		}

		if (read[0] == '=' && !bInQuotes)
		{
			bReadValue = false;
			bLastIsSpace = false;
			read++;
			continue;
		}

		if (bReadValue)
			value += read[0];
		else
			key += read[0];

		bLastIsSpace = false;
		read++;
	}

	return 1;
}

int CParser::ParseHeader(FHeaderData& data)
{
	std::ifstream stream(data.FilePath.c_str());
	if (!stream.is_open())
	{
		std::cerr << "ERROR - Unable to open header: '" << data.FilePath.c_str() << "'!\n";
		return 1;
	}

	//std::cout << "Parsing header '" << data.FilePath.c_str() << "'\n";

	bool bInComment = false;
	bool bInMacro = false;
	bool bFoundGeneratedInclude = false;

	EReadMode rm = RM_HEADER_BASE;

	CurLine = 0;
	readStage = 0;
	bPublic = false;

	std::string _line;
	FComment lastComment;
	FMacro readMacro;
	while (std::getline(stream, _line))
	{
		CurLine++;
		FString line = _line;
		if (line.IsEmpty())
			continue;

		// Find and remove comments.
		{
			SizeType slashP;
			if (bInComment)
			{
				slashP = line.Find("*/");
				if (slashP != FString::npos)
				{
					FString comment = line;
					comment.Erase(comment.begin() + slashP, comment.end());
					lastComment.text += comment;

					lastComment.text.ReplaceAll('\t', ' ');
					lastComment.text.EraseAll('*');
					lastComment.lineEnd = CurLine;

					line.Erase(line.begin(), line.begin() + slashP + 2);
					bInComment = false;
				}
				else
				{
					lastComment.text += line + "\\n";
					continue;
				}
			}

			slashP = line.Find("//");
			if (slashP != FString::npos)
				line.Erase(line.begin() + slashP, line.end());

			slashP = line.Find("/*");
			if (slashP != FString::npos)
			{
				SizeType endSlash = line.Find("*/", slashP);
				if (endSlash == FString::npos)
				{
					bInComment = true;
					lastComment.text.Clear();
					lastComment.text += line.c_str() + slashP + 2;
					lastComment.text += "\\n";
					continue;
				}
				else
				{
					FString comment = line;
					lastComment.lineEnd = CurLine;
					line.Erase(line.begin() + slashP, line.begin() + endSlash + 2);

					comment.Erase(comment.begin() + endSlash, comment.end());
					comment.Erase(comment.begin(), comment.begin() + slashP + 2);
					lastComment.text = comment;
				}
			}
		}

		// Handle inlcudes
		if (rm == RM_HEADER_BASE)
		{
			SizeType includePos = line.Find("#include");
			if (includePos != FString::npos)
			{
				if (bFoundGeneratedInclude)
				{
					std::cerr << "ERROR - generated head must be the last include in the file! line: " << CurLine << std::endl;
					return 2;
				}
				
				if (line.Find(".generated.h") != FString::npos)
					bFoundGeneratedInclude = true;

				continue;
			}
		}

		// Ignore macro definitions or anything that starts with #
		{
			if (bInMacro)
			{
				if (line[line.Size() - 1] == '\\')
					continue;

				bInMacro = false;
				continue;
			}

			SizeType macroStart = line.Find("#");
			if (macroStart != FString::npos)
			{
				SizeType isIf = line.Find("#if");
				if (isIf != -1)
				{
					IfStack.Add(FString());
					for (SizeType i = isIf + 3; i < line.Size(); i++)
					{
						if (line[i] == ' ')
							continue;
						(*IfStack.last()) += line[i];
					}

					continue;
				}
				isIf = line.Find("#endif");
				if (isIf != -1)
				{
					IfStack.PopBack();
					continue;
				}

				if (line[line.Size()-1] == '\\')
					bInMacro = true;
				continue;
			}
		}

		if (rm == RM_HEADER_BASE && TryReadMacro(line, readMacro))
		{
			rm = MacroTypeToReadMode(readMacro.type);
			readStage = 0;
			continue;
		}

		if (rm == RM_CLASS)
		{
			if (readStage == 0)
			{
				SizeType pos = stream.tellg();
				pos -= line.Size();
				stream.seekg(pos);

				data.classes.Add(CppClass());
				CppClass& _class = *data.classes.last();
				
				if (CurLine - lastComment.lineEnd < 3)
					_class.comment = lastComment.text;

				_class.classMacro = readMacro;
				_class.line = CurLine;
				if (readMacro.type == FMacro::STRUCT)
					bPublic = true;
				else
					bPublic = false;

				FString prevValue;
				FString curValue;
				bool bPrevWasSpace = false;
				while (true)
				{
					//while (readStage == 3)
					//{
					//	std::string _l;
					//	std::getline(stream, _l);
					//	
					//	if (TryReadMacro(_l, _class.bodyMacro))
					//		readStage = 4;
					//}

					char ch;
					stream.read(&ch, sizeof(char));

					if (readStage == 3)
						break;

					if (ch == '{')
					{
						readStage = 3;
						prevValue = curValue;
						curValue.Clear();
					}

					if (ch == ' ' || ch == '\t' || ch == '\n' || ch == ':' || ch == ',')
					{
						if (readStage == 0 && (ch == ':' || ch == '\n'))
						{
							_class.name = bPrevWasSpace ? prevValue : curValue;
							readStage = 1;
						}
						if (readStage == 1 && (ch == ',' || ch == '\n'))
						{
							_class.baseName = bPrevWasSpace ? prevValue : curValue;
							readStage = 2;
						}

						if (ch == ' ' || ch == '\t')
							bPrevWasSpace = true;

						if (ch == '\n')
							CurLine++;
						
						prevValue = curValue;
						curValue.Clear();
						continue;
					}

					curValue += ch;
					bPrevWasSpace = false;
				}
				continue;
			}
			if (readStage == 3)
			{
				CppClass& _class = *data.classes.last();
				if (TryReadMacro(line, _class.bodyMacro))
					readStage = 4;
				continue;
			}
			if (readStage == 4)
			{
				CppClass& _class = *data.classes.last();

				if (line.Find("};") != FString::npos)
				{
					rm = RM_HEADER_BASE;
					continue;
				}

				{
					SizeType pP = line.Find("public:");
					if (pP != -1)
					{
						bPublic = true;
						continue;
					}
					pP = line.Find("private:");
					if (pP != -1)
					{
						bPublic = false;
						continue;
					}
					pP = line.Find("protected:");
					if (pP != -1)
					{
						bPublic = false;
						continue;
					}
				}

				if (TryReadMacro(line, readMacro))
				{
					if (readMacro.type == FMacro::PROPERTY)
					{
						_class.Properties.Add(CppProperty());
						if (IfStack.Size() > 0)
							(*_class.Properties.last()).IfGuard = *IfStack.last();

						rm = RM_PROPERTY;
					}
					else if (readMacro.type == FMacro::FUNCTION)
					{
						_class.Functions.Add(CppFunction());
						if (IfStack.Size() > 0)
							(*_class.Functions.last()).IfGuard = *IfStack.last();

						rm = RM_FUNCTION;
					}
					continue;
				}
				continue;
			}
			continue;
		}
		if (rm == RM_PROPERTY)
		{
			CppClass& _class = *data.classes.last();
			CppProperty& property = *_class.Properties.last();
			property.macro = readMacro;
			property.line = CurLine;
			property.bPrivate = !bPublic;

			if (CurLine - lastComment.lineEnd < 3)
				property.comment = lastComment.text;

			bool bPrevWasSpace = false;
			FString value;
			FString prevValue;
			int expectedValue = 0; // 0 = type | static | template. 1 = ref | pointer. 2 = name.
			for (auto ch : line)
			{
				if (ch == '>')
				{
					property.typeName = value;

					expectedValue = 2;
					bPrevWasSpace = false;
					continue;
				}

				if (ch == '<' && expectedValue == 0)
				{
					if (!IsTemplateSupported(value))
					{
						std::cerr << "ERROR - Unsupported template '" << value.c_str() << "'! line:" << CurLine << "\n";
						return 3;
					}

					if (!property.templateTypename.IsEmpty())
					{
						if (!TemplateSupportsSubTemplate(property.templateTypename))
						{
							std::cerr << "error: " << property.templateTypename.c_str() << " does not support sub templates!\n";
							return 4;
						}

						property.nestedTemplateType = value;
					}
					else
						property.templateTypename = value;
					property.bTemplateType = true;

					bPrevWasSpace = false;
					value.Clear();
					continue;
				}

				if (ch == '*' && expectedValue < 2)
				{
					property.bPointer = true;
					property.typeName = value;

					expectedValue = 2;
					bPrevWasSpace = false;
					continue;
				}

				if (ch == ':' || ch == '=')
				{
					if (bPrevWasSpace)
						property.name = prevValue;
					else
						property.name = value;

					rm = RM_CLASS;
					expectedValue = 3;
					bPrevWasSpace = false;
					break;
				}

				if (ch == ';')
				{
					property.name = value;
					rm = RM_CLASS;

					expectedValue = 3;
					bPrevWasSpace = false;
					break;
				}

				if (ch == '&' && expectedValue < 2)
				{
					std::cerr << "ERROR - Object references cannot be reflected! line: " << CurLine << "\n";
					return 4;
				}

				if (ch == ' ' || ch == '\t')
				{
					if (value == "static" && expectedValue == 0)
					{
						property.bStatic = true;
					}
					else if (expectedValue == 0 && !value.IsEmpty())
					{
						property.typeName = value;
						expectedValue = 1;
					}

					if (expectedValue == 2)
					{
						property.name = value;
					}
					
					bPrevWasSpace = true;
					prevValue = value;
					value.Clear();
					continue;
				}

				value += ch;
				bPrevWasSpace = false;
			}

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

			continue;
		}
		if (rm == RM_FUNCTION)
		{
			CppClass& _class = *data.classes.last();
			CppFunction& func = *_class.Functions.last();
			func.macro = readMacro;
			func.line = CurLine;
			func.bPrivate = !bPublic;

			if (CurLine - lastComment.lineEnd < 3)
				func.comment = lastComment.text;
			
			bool bPrevWasSpace = false;
			FString prevValue;
			FString value;
			int funcRM = 0;
			CppProperty p;
			for (char ch : line)
			{
				if (ch == '(')
				{
					func.name = value;

					funcRM = 2;
					value.Clear();
					bPrevWasSpace = false;
					continue;
				}

				if (funcRM == 2 && (ch == ',' || ch == ')') && !value.IsEmpty())
				{
					SizeType lastSpace = value.FindLastOf(' ');
					prevValue = value;
					prevValue.Erase(prevValue.begin() + lastSpace, prevValue.end());
					value.Erase(value.begin(), value.begin() + lastSpace + 1);

					p.name = value;
					p.typeName = prevValue;
					if (SizeType i = p.typeName.FindLastOf('*'); i != -1)
					{
						p.bPointer = true;
						p.typeName.Erase(p.typeName.At(i));
					}

					func.Arguments.Add(p);
					p = CppProperty();
				}

				if (ch == '<' && funcRM == 2)
				{
					if (value != "TObjectPtr")
					{
						std::cerr << "ERROR: Functions cannot contain template arguments! '" << _class.name.c_str() << "::" << func.name.c_str() << "\n";
						break;
					}
					p.templateTypename = "TObjectPtr";
					p.bTemplateType = true;
					continue;
				}

				if (ch == ')')
				{
					rm = RM_CLASS;
					break;
				}

				if ((ch == ' ' || ch == '\t') && funcRM == 0)
				{
					if (!value.IsEmpty())
					{
						uint32_t keyword = IsFunctionKeyword(value);
						if (keyword == 0)
						{
							if (value != "void")
							{
								std::cerr << "ERROR - Function must return void! line: " << CurLine << "\n";
							}

							funcRM = 1;
						}
						else if (keyword == 1)
						{
							std::cerr << "ERROR - Invalid function! line: " << CurLine << "\n";
							return 5;
						}
						else if (keyword == 2)
							func.bStatic = true;
					}

					bPrevWasSpace = true;
					prevValue = value;
					value.Clear();
					continue;
				}

				bPrevWasSpace = false;
				value += ch;
			}

			CppFunction::EType type = CppFunction::GENERAL;

			if (func.macro.ArgIndex("ServerRpc") != -1)
				if (type > 0)
					goto funcErr;
				else
					type = CppFunction::SERVER_RPC;
			if (func.macro.ArgIndex("ClientRpc") != -1)
				if (type > 0)
					goto funcErr;
				else
					type = CppFunction::CLIENT_RPC;
			if (func.macro.ArgIndex("MulticastRpc") != -1)
				if (type > 0)
					goto funcErr;
				else
					type = CppFunction::MUTLICAST_RPC;
			if (func.macro.ArgIndex("Input") != -1)
				if (type > 0)
					goto funcErr;
				else
					type = CppFunction::INPUT;
			if (func.macro.ArgIndex("ConCmd") != -1)
				if (type > 0)
					goto funcErr;
				else
					type = CppFunction::COMMAND;

			func.type = type;

			goto endFunc;
		funcErr:
			std::cerr << "error: function can only be of one type line:" << std::to_string(CurLine) << " '" << data.FilePath.c_str() << "'" << std::endl;

		endFunc:
			continue;
		}
		if (rm == RM_ENUM)
		{
			if (readStage == 0)
			{
				data.enums.Add(CppEnum());
				CppEnum& _enum = *data.enums.last();

				_enum.macro = readMacro;
				_enum.line = CurLine;

				SizeType pos = stream.tellg();
				pos -= line.Size();
				stream.seekg(pos);

				FString prevValue;
				FString value;
				bool bPrevWasSpace = false;
				while (true)
				{
					char ch;
					stream.read(&ch, sizeof(ch));

					if (ch == '\n')
					{
						//CurLine++;
						if (_enum.name.IsEmpty())
						{
							_enum.name = bPrevWasSpace ? prevValue : value;
							readStage = 1;
							break;
						}
					}

					if (ch == ' ' || ch == '\t' || ch == '\n')
					{
						bPrevWasSpace = true;
						prevValue = value;
						value.Clear();
						continue;
					}

					if (ch == ':')
					{
						_enum.name = bPrevWasSpace ? prevValue : value;
						bPrevWasSpace = false;
						continue;
					}

					if (ch == '{')
					{
						readStage = 1;

						while (ch != '\n')
							stream.read(&ch, sizeof(char));
						break;
					}

					bPrevWasSpace = false;
					value += ch;
				}
				continue;
			}
			if (readStage == 1)
			{
				if (line.Find("};") != -1)
				{
					readStage = 0;
					rm = RM_HEADER_BASE;
					continue;
				}

				if (SizeType i = line.FindFirstOf('{'); i != -1)
					line.Erase(line.begin(), line.begin() + i + 1);

				if (line.Size() < 3)
					continue;

				CppEnum& _enum = *data.enums.last();
				_enum.Values.Add(FEnumVariable());
				FEnumVariable& _var = *_enum.Values.last();

				FMacro meta;
				if (TryReadMacro(line, meta) && meta.type == FMacro::META)
				{
					_var.meta = meta;
				}

				line += '\n';

				FString prevValue;
				FString value;
				bool bPrevWasSpace = false;
				for (char ch : line)
				{
					if (ch == ' ' || ch == '\t')
					{
						bPrevWasSpace = true;
						prevValue = value;
						value.Clear();
						continue;
					}

					if (ch == '=')
					{
						_var.name = bPrevWasSpace ? prevValue : value;
						continue;
					}

					if (ch == ',' || ch == '\n')
					{
						if (!_var.name.IsEmpty())
						{
							if (prevValue[0] == '=')
								_var.value = value;
						}
						else
							_var.name = bPrevWasSpace ? prevValue : value;

						bPrevWasSpace = false;
						continue;
					}

					bPrevWasSpace = false;
					value += ch;
				}
			}


			continue;
		}
	}

	data.bEmpty = data.classes.Size() == 0 && data.enums.Size() == 0;
	return 0;
}
