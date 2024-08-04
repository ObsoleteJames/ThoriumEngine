
#include "ScriptCompiler.h"
#include "Script/Token.h"
//#include "Script/ScriptClass.h"
#include "Module.h"

#include <unordered_map>
#include <set>
#include <iostream>

#define TRY(func) int r = func; if (r != 0) return r

struct FAnnotation
{
	bool bHasArgument;
	bool bClasses;
	bool bStructs;
	bool bEnums;
	bool bFunctions;
	bool bVariables;
};

static std::unordered_map<std::string, FAnnotation> mapAnnotations = {
	{ "name",			{ true, true, true, true, true, true } },
	{ "category",		{ true, true, true, true, true, true } },
	{ "description",	{ true, true, true, true, true, true } },

	{ "editable",		{ false, false, false, false, false, true } },
	{ "readOnly",		{ false, false, false, false, false, true } },

	{ "execOnInit",		{ false, false, false, false, true, false } },
	{ "execOnPostInit", { false, false, false, false, true, false } },
	{ "execOnShutdown", { false, false, false, false, true, false } },

	{ "serverRpc",		{ false, false, false, false, true, false } },
	{ "clientRpc",		{ false, false, false, false, true, false } },
	{ "mutlicastRpc",	{ false, false, false, false, true, false } },

	{ "input",			{ false, false, false, false, true, false } },
};

static std::vector<FString> mapModKeywords = {
	"template"
	"static",
	"virtual",
	"override",
	"const",
	"private",
	"public",
	"protected",
};

static std::unordered_map<FString, int> mapProtectionLvl = {
	{ "private",	0 },
	{ "public",		2 },
	{ "protected",	3 },
};

static std::unordered_map<std::string, EDataType> mapGenericTypes = {
	{ "void",		EVT_VOID },
	{ "string",		EVT_STRING },
	{ "int",		EVT_INT },
	{ "int8",		EVT_INT8 },
	{ "int16",		EVT_INT16 },
	{ "int64",		EVT_INT64 },
	{ "uint",		EVT_UINT },
	{ "uint8",		EVT_UINT8 },
	{ "uint16",		EVT_UINT16 },
	{ "uint64",		EVT_UINT64 },
	{ "float",		EVT_FLOAT },
	{ "double",		EVT_DOUBLE },
	{ "bool",		EVT_BOOL },
};

int CThScriptCompiler::CompileFile(const std::string& file, CThScript& out)
{
	CTokenizer tokenizer;

	script = &out;

	if (!tokenizer.ParseFile(file, tokens))
	{
		std::cout << "ERROR: Failed to open file '" << file << "'\n";
		return FILE_ERROR;
	}

	state.stateIndex = 0;
	state.state[0] = CS_FILE_SCOPE;

	//FCompilerState state;
	for (auto token = tokens.begin(); token != tokens.end(); token++)
	{
		//auto& token = tokens.begin() + i;

		if (token[0].type == FToken::Unkown)
		{
			//std::cout << "ERROR: Invalid token at line: " << token[0].line << " --> " << token[0].text << std::endl;
			PrintError("Invalid token!", &token[0]);
			return INVALID_TOKEN;
		}

		if (token[0].type == FToken::Keyword)
		{
			//if (mapModKeywords.find(token[0].text) != mapModKeywords.end())
			if (std::find(mapModKeywords.begin(), mapModKeywords.end(), token[0].text) != mapModKeywords.end())
			{
				if (!(mapProtectionLvl.find(token[0].text) != mapProtectionLvl.end() && token[1].text == ":"))
				{
					curKeywords.Add(token[0]);
					continue;
				}
			}
		}

		if (token[0].type == FToken::Operator)
		{
			/*if (token[0].text != "@")
			{
				PrintError("Invalid operator outside of body!", &token[0]);
				return INVALID_TOKEN;
			}*/

			if (token[0].text == "@")
			{
				token++;
				TRY(ParseAnnotation(token));
				continue;
			}
		}

		switch (state.Get())
		{
		case CS_FILE_SCOPE:
		{
			if (token[0].type == FToken::Symbol)
			{
				PrintError("Invalid symbol outside of body!", &token[0]);
				return INVALID_SYNTAX;
			}

			if (token[0].type == FToken::Keyword)
			{
				if (token[0].text == "class")
				{
					TRY(ParseClassDef(token));

					protectionLvl = 0;
					state.PushState(CS_CLASS_BODY);
					break;
				}
			}
		}
		break;

		case CS_CLASS_BODY:
		{
			if (token[0].type == FToken::ScopeOpen)
			{
				scopeIndex++;
				break;
			}

			if (token[0].type == FToken::ScopeClose)
			{
				scopeIndex--;

				if (scopeIndex == 0)
				{
					if (curAnnotations.Size() > 0)
					{
						PrintWarning("Annotations are present but no function or property was provided.", nullptr);
						curAnnotations.Clear();
					}

					ConstructClassLayout(script->classes.last());

					state.PopState();
				}

				break;
			}

			if (token[0].type == FToken::Keyword)
			{
				if (auto it = mapProtectionLvl.find(token[0].text); it != mapProtectionLvl.end())
				{
					if (token[1].type == FToken::Operator && token[1].text == ":")
					{
						protectionLvl = it->second;
						token++;
						break;
					}
				}

				// most likely defining a function.
				if (token[0].text == "void")
				{
					token--;
					state.PushState(CS_FUNCTION_DEF);
					break;
				}
			}

			// check if the current token is a data type.
			// if it is, check if either a function or property is being defined.
			FArgType valueType = GetDataType(token[0]);
			if (valueType.type != EVT_NULL)
			{
				if (token[1].type != FToken::Symbol)
				{
					//std::cout << "ERORR: invalid syntax! line: " << token[1].line << std::endl;
					PrintError("Invalid syntax!", &token[1]);
					return INVALID_SYNTAX;
				}

				if (token[2].type == FToken::ParenthesisOpen)
				{
					// this must be a function, so go into the function state.
					token--;
					state.PushState(CS_FUNCTION_DEF);
					break;
				}

				// it's not a function, so it must be a property.
				token--;
				state.PushState(CS_PROPERTY_DEF);
				break;
			}

			if (token[0].type == FToken::Symbol)
			{
				PrintError("Invalid syntax!", &token[0]);
				return INVALID_SYNTAX;
			}
		}
		break;

		case CS_FUNCTION_DEF:
		{
			FArgType returnType = GetDataType(token[0]);

			if (returnType.type == EVT_NULL)
			{
				//std::cout << "ERROR: Invalid return type for function! line: " << token[0].line << std::endl;
				PrintError("Invalid return type for function!", &token[0]);
				return INVALID_SYNTAX;
			}

			if (token[1].type != FToken::Symbol)
			{
				//std::cout << "ERORR: invalid syntax! line: " << token[1].line << std::endl;
				PrintError("Invalid syntax!", &token[1]);
				return INVALID_SYNTAX;
			}

			FString funcName = token[1].text;
			token = token + 2;

			if (token[0].type != FToken::ParenthesisOpen)
			{
				//std::cout << "EROR: Invalid syntax, expected '('! line: " << token[0].line << std::endl;
				PrintError("Invalid syntax, expected '('!", &token[0]);
				return INVALID_SYNTAX;
			}

			FScriptFunction* meta = new FScriptFunction();
			meta->name = funcName;
			meta->protectionLvl = protectionLvl;
			meta->bStatic = script->classes.back()->meta->bIsStatic;
			meta->returnType = returnType;

			//FThsFunction func{};
			//func.meta = meta;
			//func.parent = script->classes.back();

			for (auto& keyw : curKeywords)
			{
				if (auto it = mapProtectionLvl.find(keyw.text); it != mapProtectionLvl.end())
				{
					meta->protectionLvl = it->second;
				}
				else if (keyw.text == "static")
				{
					meta->bStatic = true;
				}
				else if (keyw.text == "virtual")
				{
					meta->bVirtual = true;
				}
				else if (keyw.text == "override")
				{
					meta->bVirtual = true;
					meta->bOverride = true;
				}
				else
				{
					PrintError("Cannot state property as " + keyw.text + "!", nullptr);
					return INVALID_SYNTAX;
				}
			}
			curKeywords.clear();

			for (auto& ann : curAnnotations)
			{
				if (ann.first == "name")
					meta->displayName = ann.second;
				else
					meta->metaData[ann.first] = { ann.first, ann.second };
			}
			curAnnotations.clear();

			script->classes.back()->AddFunction(meta);
			token++;

			while (true)
			{
				if (token[0].type == FToken::ParenthesisClose)
					break;

				FArgType type = GetDataType(token[0]);
				if (type.type == EGenericType::Null)
				{
					PrintError("Invalid argument type for function!", &token[0]);
					return INVALID_SYNTAX;
				}

				token++;

				if (token[0].type != FToken::Symbol)
				{
					PrintError("Invalid syntax!", &token[0]);
					return INVALID_SYNTAX;
				}

				std::string name = token[0].text;

				token++;
				if (token[0].type != FToken::Seperator)
				{
					PrintError("Invalid syntax, expected ','!", &token[0]);
					return INVALID_SYNTAX;
				}

				meta->arguments.push_back({ name, type });
				token++;
			}

			functions.push_back({});
			functions.back().function = script->classes.back()->functions.back();
			state.SetState(CS_FUNCTION_BODY);
		}
		break;

		case CS_FUNCTION_BODY:
		{
			int scopeLevel = scopeIndex;
			while (true)
			{
				if (token[0].type == FToken::ScopeOpen)
					scopeIndex++;

				functions.back().tokens.push_back(token[0]);

				if (token[0].type == FToken::ScopeClose)
				{
					scopeIndex--;

					if (scopeIndex == scopeLevel)
						state.PopState();

					break;
				}
				token++;
			}
		}
		break;

		case CS_PROPERTY_DEF:
		{
			FArgType type = GetDataType(token[0]);

			if (type.type == EGenericType::Null || type.type == EGenericType::Void)
			{
				//std::cout << "ERROR: Invalid return type for function! line: " << token[0].line << std::endl;
				PrintError("Invalid type!", &token[0]);
				return INVALID_SYNTAX;
			}

			if (token[1].type != FToken::Symbol)
			{
				//std::cout << "ERORR: invalid syntax! line: " << token[1].line << std::endl;
				PrintError("Invalid syntax!", &token[1]);
				return INVALID_SYNTAX;
			}

			std::string name = token[1].text;
			std::string description;
			std::string category;
			token += 2;

			FToken value{};
			bool bIsArray = false;
			bool bIsStatic = script->classes.back()->meta->bIsStatic;
			int arraySize = 0; // default array size.
			int protection = protectionLvl;

			// type is an array.
			if (token[0].type == FToken::BracketOpen)
			{
				bIsArray = true;

				token++;
				if (token[0].type == FToken::NumericLiteral)
				{
					arraySize = (int)token[0].value;
					token++;
				}

				if (token[0].type != FToken::BracketClose)
				{
					PrintError("Invalid syntax, expected ']'!", &token[0]);
					return INVALID_SYNTAX;
				}

				token++;
			}

			if (token[0].type == FToken::Operator && token[0].text == "=")
			{
				token++;
				value = token[0];
				token++;
			}

			if (token[0].type != FToken::EndOfInstruction)
			{
				PrintError("Invalid syntax, expected ';'!", &token[0]);
				return INVALID_SYNTAX;
			}

			for (auto& keyword : curKeywords)
			{
				if (auto it = mapProtectionLvl.find(keyword.text); it != mapProtectionLvl.end())
				{
					protection = it->second;
				}
				else if (keyword.text == "static")
				{
					bIsStatic = true;
				}
				else
				{
					PrintError("Cannot state property as " + keyword.text + "!", nullptr);
					return INVALID_SYNTAX;
				}
			}
			curKeywords.clear();

			FProperty* meta = new FProperty();
			meta->name = name;
			meta->displayName = name;
			meta->id = std::hash<std::string>{}(name);
			meta->protectionLvl = protection;
			meta->type = bIsArray ? EGenericType::Array : type.type;
			meta->typeId = type.typeId;
			if (bIsArray)
			{
				meta->templateType = type.type;
				meta->templateTypeId = type.typeId;
			}

			FThsProperty property{};
			property.defaultValue = value.text;
			property.meta = meta;
			property.parent = script->classes.back();
			//script->classes.back()->properties.push_back(property);
			script->classes.back()->AddProperty(property);

			for (auto& ann : curAnnotations)
			{
				if (ann.first == "name")
					meta->displayName = ann.second;
				//else if (ann.first == "description")
					//meta->;
				//else if (ann.first == "category")
				//	category = ann.second;
				//else if (ann.first == "readOnly")

				meta->metaData[ann.first] = { ann.first, ann.second };
			}
			curAnnotations.clear();

			state.PopState();
		}
		break;
		}
	}

	new CScriptModule(script, file.c_str());
	return 0;
}

int CThScriptCompiler::ParseAnnotation(FTokenIt& token)
{
	if (token[0].text == "import")
	{
		if (state.Get() != CS_FILE_SCOPE)
		{
			PrintError("Cannot use import here!", &token[0]);
			return INVALID_SYNTAX;
		}

		//if (token[1].type != FToken::Operator && token[1].text != "=")
		//{
		//	std::cout << "ERROR: Invalid use of @import! must provide a library name! line: " << token[1].line << std::endl;
		//	return INVALID_SYNTAX;
		//}

		if (token[1].type != FToken::Symbol && token[1].type != FToken::StringLiteral)
		{
			PrintError("Invalid use of @import! Expected library name.", &token[1]);
			return INVALID_SYNTAX;
		}

		if (token[2].type != FToken::EndOfInstruction)
		{
			PrintError("Expected ';' at", &token[2]);
			return INVALID_SYNTAX;
		}

		ImportModule(token[1].text);

		token += 2;
		return 0;
	}

	if (auto it = mapAnnotations.find(token[0].text); it != mapAnnotations.end())
	{
		if (it->second.bHasArgument)
		{
			if (token[1].type != FToken::Operator && token[1].text != "=")
			{
				std::cout << "ERROR: Invalid annotation, requires argument! line: " << token[0].line << " --> " << token[0].text << std::endl;
				return INVALID_SYNTAX;
			}

			if (token[2].type != FToken::Symbol && token[2].type != FToken::StringLiteral)
			{
				std::cout << "ERROR: Invalid annotation! line: " << token[2].line << std::endl;
				return INVALID_SYNTAX;
			}

			if (token[3].type != FToken::EndOfInstruction)
			{
				std::cout << "ERROR: Expected ';' at line: " << token[3].line << std::endl;
				return INVALID_SYNTAX;
			}

			curAnnotations.push_back({ token[0].text, token[2].text });
			token += 3;
		}
		else
		{
			if (token[1].type != FToken::EndOfInstruction)
			{
				std::cout << "ERROR: Expected ';' at line: " << token[3].line << std::endl;
				return INVALID_SYNTAX;
			}

			curAnnotations.push_back({ token[0].text, std::string() });
			token++;
		}
	}
	else
	{
		std::cout << "ERROR: Invalid annotation! line: " << token[0].line << " --> " << token[0].text << std::endl;
		return INVALID_SYNTAX;
	}

	return 0;
}

int CThScriptCompiler::ParseClassDef(FTokenIt& token)
{
	if (token[1].type != FToken::Symbol)
	{
		if (token[1].text == "static")
		{
			PrintError("Invalid syntax, 'static', must be defined before 'class'!", &token[1], false);
			return INVALID_SYNTAX;
		}

		PrintError("Invalid syntax, expected class name!", &token[1]);
		return INVALID_SYNTAX;
	}

	bool bStatic = false;
	//if (token[-1].type == FToken::Keyword && token[-1].text == "static")
	//	bStatic = true;

	for (auto& it : curKeywords)
	{
		if (it.text != "static")
		{
			PrintError("cannot state class as" + it.text + "!", &token[0], false);
			return INVALID_SYNTAX;
		}
		else
			bStatic = true;
	}
	curKeywords.clear();

	std::string className;
	std::string baseClass;

	className = token[1].text;
	token += 2;

	FClass* base = nullptr;

	if (token[0].type == FToken::Operator && token[0].text == ":")
	{
		if (token[1].type != FToken::Symbol)
		{
			PrintError("Invalid syntax, expected class name!", &token[1]);
			return INVALID_SYNTAX;
		}

		baseClass = token[1].text;

		base = CModuleManager::GetClass(baseClass);
		if (!base)
		{
			PrintError("Unkown class '" + baseClass + "'!", &token[1]);
			return INVALID_DATA;
		}

		token += 2;
	}

	if (token[0].type != FToken::ScopeOpen)
	{
		PrintError("Invalid syntax, epxected '{'!", &token[0]);
		return INVALID_SYNTAX;
	}

	auto* c = new CThsClass();
	c->meta = new FClass();
	c->meta->name = className;
	c->meta->displayName = className;
	c->meta->baseTypeId = base ? base->id : 0;
	c->meta->bIsStatic = bStatic;
	c->meta->bIsScriptType = true;
	c->meta->type = EDataType::Class;
	c->meta->bIsTemplate = false;
	c->meta->id = std::hash<std::string>{}(className);

	for (auto& ann : curAnnotations)
	{
		if (ann.first == "name")
			c->meta->displayName = ann.second;
		else
			c->meta->metaData[ann.first] = { ann.first, ann.second };
	}

	script->classes.push_back(c);

	scopeIndex++;
	return 0;
}

int CThScriptCompiler::ConstructClassLayout(CThsClass* c)
{
	// first figure out the sizes of all properties
	for (auto& p : c->properties)
	{
		FProperty* prop = p.meta;

		switch (prop->type)
		{
		case EGenericType::Class:
			// Object PTR
			prop->size = 8;
			break;
		case EGenericType::Struct:
		{
			FStruct* s = CModuleManager::GetStruct(prop->typeId);
			prop->size = s->size;
		}
		break;
		case EGenericType::Enum:
			prop->size = 4;
			break;
		case EGenericType::String:
			prop->size = sizeof(std::string);
			break;
		case EGenericType::Array:
			prop->size = sizeof(std::vector<void*>);
			break;
		case EGenericType::ClassType:
			prop->size = 8;
			break;
		case EGenericType::StructType:
			prop->size = 8;
			break;
		case EGenericType::EnumType:
			prop->size = 8;
			break;
		case EGenericType::Int:
			prop->size = 4;
			break;
		case EGenericType::Int8:
			prop->size = 1;
			break;
		case EGenericType::Int16:
			prop->size = 2;
			break;
		case EGenericType::Int64:
			prop->size = 8;
			break;
		case EGenericType::Uint:
			prop->size = 4;
			break;
		case EGenericType::Uint8:
			prop->size = 1;
			break;
		case EGenericType::Uint16:
			prop->size = 2;
			break;
		case EGenericType::Uint64:
			prop->size = 8;
			break;
		case EGenericType::Float:
			prop->size = 4;
			break;
		case EGenericType::Double:
			prop->size = 8;
			break;
		case EGenericType::Bool:
			prop->size = 1;
			break;
		}
	}

	if (c->meta->bIsStatic)
		return 0;

	// sort the list of properties by size (descending).
	std::multimap<size_t, FProperty*, std::greater<size_t>> propsSet;
	for (auto& p : c->properties)
		propsSet.insert({ p.meta->size, p.meta });

	size_t curOffset = c->meta->baseTypeId == 0 ? 0 : CModuleManager::GetClass(c->meta->baseTypeId)->size;
	for (auto it = propsSet.begin(); it != propsSet.end(); it++)
	{
		it->second->offset = curOffset;

		curOffset += it->first;
	}

	// the offset always ends up being the offset of the last property + its size.
	// therefore its equal to the size of the class.
	c->meta->size = curOffset;
	return 0;
}

void CThScriptCompiler::ImportModule(const std::string& name)
{
	auto m = CModuleManager::GetModule(name);
	if (!m)
	{
		std::cout << "WARNING: failed to import module '" << name << "!\n";
		return;
	}

	importedModules.push_back(m);
}

FClass* CThScriptCompiler::FindClass(const std::string& name)
{
	auto* r = CModuleManager::GetClass(name);
	if (!r)
	{
		for (auto& c : script->classes)
		{
			if (c->meta->name == name)
				return c->meta;
		}
	}
	return r;
}

FStruct* CThScriptCompiler::FindStruct(const std::string& name)
{
	auto* r = CModuleManager::GetStruct(name);
	if (!r)
	{
		for (auto& c : script->structs)
		{
			if (c->name == name)
				return c;
		}
	}
	return r;
}

FEnum* CThScriptCompiler::FindEnum(const std::string& name)
{
	auto* r = CModuleManager::GetEnum(name);
	if (!r)
	{
		for (auto& c : script->enums)
		{
			if (c->name == name)
				return c;
		}
	}
	return r;
}

FArgType CThScriptCompiler::GetDataType(FToken& token)
{
	if (auto it = mapGenericTypes.find(token.text); it != mapGenericTypes.end())
	{
		FArgType r{};
		r.type = it->second;
		return r;
	}

	auto* c = FindClass(token.text);
	if (c)
	{
		FArgType r{};
		r.type = EGenericType::Class;
		r.typeId = c->id;
		r.bPointer = true;
		return r;
	}

	auto* s = FindStruct(token.text);
	if (s)
	{
		FArgType r{};
		r.type = EGenericType::Struct;
		r.typeId = s->id;
		return r;
	}

	auto* e = FindEnum(token.text);
	if (e)
	{
		FArgType r{};
		r.type = EGenericType::Enum;
		r.typeId = e->id;
		return r;
	}

	return { EGenericType::Null };
}

void CThScriptCompiler::PrintError(const std::string& error, FToken* token, bool bIncludeText /*= true*/)
{
	std::cout << "ERROR: " << error;
	if (token)
	{
		std::cout << " Line: " << token->line;
		if (bIncludeText)
			std::cout << " --> '" << token->text << "'";
	}

	std::cout << std::endl;
}

void CThScriptCompiler::PrintWarning(const std::string& warning, FToken* token, bool bIncludeText /*= true*/)
{
	std::cout << "WARNING: " << warning;
	if (token)
	{
		std::cout << " Line: " << token->line;
		if (bIncludeText)
			std::cout << " --> " << token->text;
	}

	std::cout << std::endl;
}
