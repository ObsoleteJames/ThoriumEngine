
#include "Token.h"

#include <array>
#include <unordered_map>
#include <map>
#include <fstream>

static std::unordered_map<std::string, FOperator> operators = {
	{ "*", { 3, 2 } },
	{ "/", { 3, 2 } },
	{ "+", { 1, 2 } },
	{ "-", { 1, 2 } },

	{ "=", { 1, 2 } },
	{ "==", { 1, 2 } },
	{ ":", { 1, 2 } },
	{ ".", { 1, 2 } },
	{ "@", { 1, 2 } },
	//{ "<<", { 1, 2 } },
	//{ ">>", { 1, 2 } },
	{ "<", { 1, 2 } },
	{ ">", { 1, 2 } },
	{ "^", { 1, 2 } },

	{ "\\", { 1, 2 } },
	{ "&", { 1, 2 } },
	{ "|", { 1, 2 } },
	{ "~", { 1, 2 } },
	{ "%", { 1, 2 } },
	{ "&&", { 1, 2 } },
	{ "||", { 1, 2 } },

	{ "!", { 1, 1 } },
	{ "@", { 1, 1 } },
	{ "?", { 1, 2 } },

	// unary uperators
	{ "u+", { 100, 1 } },
	{ "u-", { 100, 1 } },
};

static std::unordered_map<char, std::string> escapeChars = {
	{ 'n',  "\n"  },
	{ 't',  "\t"  },
	{ '\"', "\"" },
	{ '\\', "\\" },
	{ '?', "\?" },
	{ 't',  "\t"  },
	{ '\'', "\'" },
	{ 'r', "\r" },
	{ 'b', "\b" },
	{ 'a', "\a" },
	{ 'f', "\f" },
	{ 'v', "\v" },
	{ '0', "\0" },
};

static std::unordered_map<std::string, FKeyword> keywords = {
	{ "class",		{ FKeyword::Class } },
	{ "struct",		{ FKeyword::Struct } },
	{ "void",		{ FKeyword::Void } },
	{ "template",	{ FKeyword::Template } },
	{ "enum",		{ FKeyword::Enum } },
	{ "static",		{ FKeyword::Static } },
	{ "virtual",	{ FKeyword::Virtual } },
	{ "override",	{ FKeyword::Override } },
	{ "operator",	{ FKeyword::Operator } },
	{ "const",		{ FKeyword::Const } },
	{ "if",			{ FKeyword::If } },
	{ "else",		{ FKeyword::Else } },
	{ "for",		{ FKeyword::For } },
	{ "while",		{ FKeyword::While } },
	{ "continue",	{ FKeyword::Continue } },
	{ "break",		{ FKeyword::Break } },
	{ "return",		{ FKeyword::Return } },
	{ "private",	{ FKeyword::Private } },
	{ "public",		{ FKeyword::Public } },
	{ "protected",	{ FKeyword::Protected } },
	{ "inline",		{ FKeyword::Inline } },
};

static constexpr std::array<bool, 256> MakeLUT(const std::string_view& str)
{
	std::array<bool, 256> r{ 0 };
	for (const auto& c : str)
		r.at((uint8_t)c) = true;
	return r;
}

static constexpr auto lutWhiteSpace = MakeLUT(" \t\n\r\v\f");
static constexpr auto lutNumericDigits = MakeLUT("0123456789");
static constexpr auto lutRealNumericDigits = MakeLUT("0.123456789");
static constexpr auto lutOperators = MakeLUT("@!%^&|*+-=?/\\<>~:.");
static constexpr auto lutSymbolChars = MakeLUT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789#");
static constexpr auto lutNumericModifiers = MakeLUT("lLuUxXfFbB");

bool CTokenizer::Parse(const std::string &src, std::vector<FToken> &outTokens)
{
	if (src.empty())
		return false;

	tokenBuff = &outTokens;
	it = src.begin();
	state = STATE_NewToken;
	nextState = STATE_NewToken;

	while (it != src.end())
	{
		switch (state)
		{
		case STATE_NewToken:
			DoNewToken();
			break;
		case STATE_PushToken:
			DoPushToken();
			break;
		case STATE_NumericLiteral:
		{
			if (lutRealNumericDigits[it[0]])
			{
				if (it[0] == '.')
				{
					if (!bHasDecimal)
						bHasDecimal = true;
					else
						throw std::exception("[PARSER] Invalid numeric literal, mutliple decimal points found!");
				}

				curTokenStr += it[0];
				++it;
			}
			else
			{
				if (!lutNumericModifiers[it[0]] && lutSymbolChars[it[0]])
					throw std::exception("[PARSER] invalid symbol, symbols cannot begin with a numeric digit!");

				nextState = STATE_PushToken;
				curToken = { FToken::NumericLiteral, curTokenStr, std::stod(curTokenStr), curLine };
			}
		}
			break;
		case STATE_StringLiteral:
		{
			if (it[0] == '\\')
			{
				bNextIsEscapeChar = true;
				++it;
			}
			else if (it[0] == curStringDelimiter && !bNextIsEscapeChar)
			{
				++it;
				nextState = STATE_PushToken;
				curToken = { FToken::StringLiteral, curTokenStr, 0, curLine };
			}
			else
			{
				if (bNextIsEscapeChar)
				{
					//curTokenStr += escapeChars.at(it[0]);
					if (auto escIt = escapeChars.find(it[0]); escIt != escapeChars.end())
						curTokenStr += escIt->second;
					
					bNextIsEscapeChar = false;
					++it;
				}
				else
				{
					curTokenStr += it[0];
					++it;
				}
			}
		}
			break;
		case STATE_Symbol:
		{
			if (lutSymbolChars[it[0]])
			{
				curTokenStr += it[0];
				++it;
			}
			else if (!curTokenStr.empty())
			{
				if (keywords.find(curTokenStr) != keywords.end())
					curToken = { FToken::Keyword, curTokenStr, 0, curLine };
				else
					curToken = { FToken::Symbol, curTokenStr, 0, curLine };

				nextState = STATE_PushToken;
			}
			else
				throw std::exception("[PARSER] Unkown symbol!");
		}
			break;
		case STATE_Operator:
		{
			if (lutOperators[it[0]])
			{
				if (operators.find(curTokenStr + it[0]) != operators.end())
				{
					curTokenStr += it[0];
					++it;
				}
				else
				{
					if (operators.find(curTokenStr) != operators.end())
					{
						curToken = { FToken::Operator, curTokenStr, 0, curLine };
						nextState = STATE_PushToken;
					}
					else
					{
						curTokenStr += it[0];
						++it;
					}
				}
			}
			else
			{
				if (operators.find(curTokenStr) != operators.end())
				{
					curToken = { FToken::Operator, curTokenStr, 0, curLine };
					nextState = STATE_PushToken;
				}
				else
					throw std::exception("[PARSER] Invalid operator!");
			}
		}
			break;
		case STATE_ParenthOpen:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::ParenthesisOpen, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
		break;
		case STATE_ParenthClose:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::ParenthesisClose, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
		break;
		case STATE_ScopeOpen:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::ScopeOpen, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
		break;
		case STATE_ScopeClose:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::ScopeClose, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
		break;
		case STATE_BracketOpen:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::BracketOpen, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
		break;
		case STATE_BracketClose:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::BracketClose, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
		break;
		case STATE_EndOfInstruction:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::EndOfInstruction, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
		break;
		case STATE_Comma:
		{
			curTokenStr += it[0];
			++it;
			curToken = { FToken::Seperator, curTokenStr, 0, curLine };
			nextState = STATE_PushToken;
		}
			break;
		}

		state = nextState;
	}
	if (state == STATE_PushToken)
		DoPushToken();

	return true;
}

bool CTokenizer::ParseFile(const std::string &file, std::vector<FToken> &outTokens)
{
	std::ifstream stream(file);
	if (!stream.is_open())
		return false;

	std::string l;
	bool bInComment = false;
	while (std::getline(stream, l))
	{
		if (bInComment)
		{
			curLine++;
			if (auto it = l.find("*/"); it != -1)
			{
				bInComment = false;
				l.erase(l.begin(), l.begin() + it + 2);
			}
			else
				continue;
		}

		if (auto it = l.find("//"); it != -1)
			l.erase(l.begin() + it, l.end());

		if (auto it = l.find("/*"); it != -1)
		{
			size_t endIt = l.find("*/", it + 2);
			if (endIt == -1)
			{
				l.erase(l.begin() + it, l.end());
				bInComment = true;
				continue;
			}
			else
				l.erase(l.begin() + it, l.begin() + endIt + 2);
		}

		if (!Parse(l + "\n", outTokens) && !l.empty())
			return false;
	}

	return true;
}

std::string CTokenizer::TokenTypeToString(FToken::Type type)
{
	const char* names[] = {
		"Unkown",
		"Operator",
		"Keyword",
		"Symbol",
		"Seperator",
		"End Of Instruction",
		"String Literal",
		"Numeric Literal",
		"(",
		")",
		"{",
		"}",
		"[",
		"]"
	};
	return names[(int)type];
}

FKeyword::Type CTokenizer::KeywordType(const std::string& text)
{
	if (keywords.find(text) != keywords.end())
		return keywords.at(text).type;

	return FKeyword::Invalid;
}

void CTokenizer::DoNewToken()
{
	curTokenStr.clear();
	curToken = { FToken::Unkown, "", 0.0, curLine };
	bHasDecimal = false;
	bNextIsEscapeChar = false;

	if (it[0] == '\n')
		curLine++;

	if (lutWhiteSpace[it[0]])
		++it;
	else if (lutNumericDigits[it[0]])
		nextState = STATE_NumericLiteral;
	else if (lutOperators[it[0]])
		nextState = STATE_Operator;
	else if (it[0] == '(')
		nextState = STATE_ParenthOpen;
	else if (it[0] == ')')
		nextState = STATE_ParenthClose;
	else if (it[0] == '{')
		nextState = STATE_ScopeOpen;
	else if (it[0] == '}')
		nextState = STATE_ScopeClose;
	else if (it[0] == '[')
		nextState = STATE_BracketOpen;
	else if (it[0] == ']')
		nextState = STATE_BracketClose;
	else if (it[0] == ',')
		nextState = STATE_Comma;
	else if (it[0] == ';')
		nextState = STATE_EndOfInstruction;
	else if (it[0] == '"' || it[0] == '\'')
	{
		curStringDelimiter = it[0];
		nextState = STATE_StringLiteral;
		++it;
	}
	else
		nextState = STATE_Symbol;
}

void CTokenizer::DoPushToken()
{
	tokenBuff->push_back(curToken);
	nextState = STATE_NewToken;
}
