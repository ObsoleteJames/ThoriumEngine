#pragma once

#include <string>
#include <Util/Core.h>

struct FToken
{
	enum Type
	{
		Unkown,
		Operator,
		Keyword,
		Symbol,
		Seperator,
		EndOfInstruction,
		StringLiteral,
		NumericLiteral,
		ParenthesisOpen,
		ParenthesisClose,
		ScopeOpen,
		ScopeClose,
		BracketOpen,
		BracketClose
	};

	Type type = Unkown;
	FString text;
	double value;

	int line;
};

struct FOperator
{
	uint8 precedence;
	uint8 numArgs;
};

class CTokenizer
{
public:
	bool Parse(const FString& src, TArray<FToken>& outTokens);
	bool ParseFile(const FString& file, TArray<FToken>& outTokens);

	static FString TokenTypeToString(FToken::Type type);

private:
	void DoNewToken();
	void DoPushToken();

private:
	enum ETokenizerState
	{
		STATE_NewToken,
		STATE_PushToken,
		STATE_StringLiteral,
		STATE_NumericLiteral,
		STATE_Symbol,
		STATE_ParenthOpen,
		STATE_ParenthClose,
		STATE_ScopeOpen,
		STATE_ScopeClose,
		STATE_BracketOpen,
		STATE_BracketClose,
		STATE_EndOfInstruction,
		STATE_Comma,
		STATE_Operator,
	};

	TArray<FToken>* tokenBuff;

	ETokenizerState state = STATE_NewToken;
	ETokenizerState nextState = STATE_NewToken;

	std::string curTokenStr;
	FToken curToken;

	std::string::const_iterator it;

	int curLine = 1;
	bool bNextIsEscapeChar = false;
	bool bHasDecimal = false;
};
