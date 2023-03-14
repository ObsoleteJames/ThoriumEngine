#pragma once

#include <Util/Core.h>

struct CppField
{
	FString name;
	FString comment; // Description of this class
	uint32_t line;

	FString IfGuard;
};

struct FMacro
{
	enum EType : uint8
	{
		STRUCT,
		CLASS,
		ENUM,
		PROPERTY,
		FUNCTION,
		GENERATED_BODY,
		ASSET,
		META,

		ETYPE_END
	};

	EType type;
	uint32_t line;

	TArray<TPair<FString, FString>> Arguments;

	SizeType ArgIndex(const FString& key)
	{
		for (SizeType i = 0; i < Arguments.Size(); i++)
			if (Arguments[i].Key == key)
				return i;

		return -1;
	}
};

struct FEnumVariable
{
	FString name;
	FString value;
	FMacro meta;
};

struct CppEnum : public CppField
{
	FMacro macro;

	TArray<FEnumVariable> Values;
};

struct CppProperty : public CppField
{
	FString typeName;
	FString templateTypename;
	FString nestedTemplateType;
	FMacro macro;

	FString fullTypename;

	bool bPrivate : 1;
	bool bPointer : 1;
	bool bTemplateType : 1;
	bool bStatic : 1;
};

struct CppFunction : public CppField
{
	enum EType
	{
		GENERAL,
		INPUT,
		COMMAND,
		SERVER_RPC,
		CLIENT_RPC,
		MUTLICAST_RPC
	};

	FMacro macro;

	// TODO: Change this to CppProperty
	TArray<CppProperty> Arguments;

	EType type;

	bool bPrivate : 1;
	bool bStatic : 1;
};

struct CppClass : public CppField
{
	FString baseName;

	TArray<CppProperty> Properties;
	TArray<CppFunction> Functions;

	FMacro bodyMacro;
	FMacro classMacro;

	CppFunction* GetFunction(const FString& name);
};
