#pragma once

#include <Util/Core.h>
#include <vector>
#include <string>

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
    std::string text;
    double value;

    int line;
};

struct FOperator
{
    uint8_t precedence;
    uint8_t numArgs;
};

struct FKeyword
{
    enum Type
    {
        Invalid,
        Class,
        Struct,
        Void,
        Template,
        Enum,
        Static,
        Virtual,
        Override,
        Operator,
        Const,
        If,
        Else,
        For,
        While,
        Continue,
        Break,
        Return,
        Private,
        Public,
        Protected,
        Inline
    };

    Type type = Invalid;
};

class CTokenizer
{
public:
    bool Parse(const std::string& src, std::vector<FToken>& outTokens);
    bool ParseFile(const std::string& file, std::vector<FToken>& outTokens);

    static std::string TokenTypeToString(FToken::Type type);
    static FKeyword::Type KeywordType(const std::string& text);

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

    std::vector<FToken>* tokenBuff;

    ETokenizerState state = STATE_NewToken;
    ETokenizerState nextState = STATE_NewToken;

    std::string curTokenStr;
    FToken curToken;

    std::string::const_iterator it;

    int curLine = 1;
    bool bNextIsEscapeChar = false;
    bool bHasDecimal = false;
	char curStringDelimiter = '"';
};
