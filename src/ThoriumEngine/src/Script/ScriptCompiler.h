#pragma once

#include "Token.h"
#include "Script.h"

enum ECompilerState
{
	CS_FILE_SCOPE, // not in any data structure.
	CS_CLASS_BODY,
	CS_STRUCT_BODY,
	CS_ENUM_BODY,
	CS_FUNCTION_DEF,
	CS_FUNCTION_BODY,
	CS_PROPERTY_DEF,
	CS_SYMBOL_DEF,
};

struct FCompilerState
{
public:
	inline void PushState(ECompilerState s)
	{
		stateIndex++;
		state[stateIndex] = s;
	}

	inline void SetState(ECompilerState s)
	{
		state[stateIndex] = s;
	}

	inline void PopState()
	{
		stateIndex--;
	}

	inline ECompilerState Get() const { return state[stateIndex]; }

	int stateIndex;
	ECompilerState state[32];
};

struct FFunctionTokens
{
	FScriptFunction* function;
	std::vector<FToken> tokens;
};

class CThScriptCompiler
{
	typedef TIterator<FToken> FTokenIt;

public:
	enum EErrorCode
	{
		SUCCESS,
		FILE_ERROR,
		INVALID_TOKEN,
		INVALID_SYNTAX,
		INVALID_DATA,
	};

public:
	CThScriptCompiler() = default;

	int CompileFile(const std::string& file, CThScript& out);

private:
	int ParseAnnotation(FTokenIt& token);

	int ParseClassDef(FTokenIt& token);

	int ConstructClassLayout(CThsClass* Class);

	void ImportModule(const std::string& name);

	// gets the class either from another module or from this script.
	FClass* FindClass(const std::string& name);
	FStruct* FindStruct(const std::string& name);
	FEnum* FindEnum(const std::string& name);

	// checks if the token is a data type, such as int or FVector.
	FArgType GetDataType(FToken& token);

	// Prints Error
	// - IndcludeText - wether to include the token text.
	void PrintError(const std::string& error, FToken* token, bool bIncludeText = true);
	void PrintWarning(const std::string& warning, FToken* token, bool bIncludeText = true);

private:
	CThScript* script;

	int scopeIndex = 0;
	int protectionLvl = 0;

	TArray<FToken> tokens;
	FCompilerState state;

	// list of functions to be compiled
	TArray<FFunctionTokens> functions;

	// list of keywords such as 'static', 'public' or 'virtual'
	// used when defining data types, functions or variables.
	TArray<FToken> curKeywords;

	TArray<std::pair<std::string, std::string>> curAnnotations;
	TArray<CModule*> importedModules;
};
