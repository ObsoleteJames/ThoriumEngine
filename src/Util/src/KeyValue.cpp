
#include <fstream>
#include <string>
#include "Util/KeyValue.h"

// 2 byte value
#define KEY_VALUE_SIGNITURE 0x36D6
const char* KV_VERSION = "KV_BINARY_2.00";

KVCategory::~KVCategory()
{
	for (auto& c : Categories)
		delete c;

	Categories.Clear();
	Values.Clear();
}

KVCategory* KVCategory::GetCategory(const FString& name, bool cie)
{
	auto it = Categories.Find([=](auto i) { return i->GetName() == name; });
	if (it != Categories.end())
		return *it;

	if (cie)
	{
		KVCategory* c = new KVCategory();
		c->Parent = this;
		c->Name = name;
		Categories.Add(c);
		return c;
	}

	return nullptr;
}

KVValue* KVCategory::GetValue(const FString& key, bool cie)
{
	for (auto& t : Values)
		if (t.Key == key)
			return &t.Value;

	if (cie)
	{
		Values.Add({ key, KVValue() });
		return &Values.At(Values.Size() - 1)->Value;
	}
	return nullptr;
}

TArray<FString>* KVCategory::GetArray(const FString& name, bool CreateIfEmpty /*= false*/)
{
	for (auto& it : arrays)
	{
		if (it.Key == name)
			return &it.Value;
	}

	if (CreateIfEmpty)
	{
		arrays.Add({ name, {} });
		return &(arrays.last()->Value);
	}

	return nullptr;
}

FKeyValue::FKeyValue(const WString& file, const EKeyValueType& type)
{
	Open(file, type);
}

bool FKeyValue::Open(const WString& file, const EKeyValueType& type)
{
	_file = file;

	if (type == KV_BINARY)
		return _tryReadBinary();
	if (type == KV_STANDARD_ASCII)
		return _tryReadAscii();

	if (!_tryReadBinary())
		return _tryReadAscii();

	return true;
}

#define WRITE_TABS() { FString __tabw = "";\
for (int i = 0; i < tabs; i++)\
	__tabw += "	";\
stream.write(__tabw.Data(), __tabw.Size()); }

bool FKeyValue::Save(const WString& file, const EKeyValueType& type) const
{
	if (bReadOnly)
		return false;

	if (type == KV_BINARY)
	{
		CFStream stream(file.c_str(), L"wb");

		int sign = KEY_VALUE_SIGNITURE;
		
		stream.Write(&sign, sizeof(int));
		int len = (int)strnlen_s(KV_VERSION, 15);
		stream.Write((char*)KV_VERSION, len);

		_saveBinary(stream);
		//stream.close();
		return true;
	}
	else if (type == KV_UNKOWN)
		return false;

	std::ofstream stream(file.c_str());
	if (!stream.is_open())
		return false;

	int tabs = 0;

	_save(stream, tabs);

	stream.close();
	return true;
}

bool FKeyValue::Save(const EKeyValueType& type) const
{
	return Save(_file, type);
}

void FKeyValue::DefineMacro(const FString& macro, int value)
{
	bReadOnly = true;
	auto it = macros.Find([=](auto i) { return i.Key == macro; });
	if (it != macros.end())
		it->Value = value;
	else
		macros.Add({ macro, value });
}

bool FKeyValue::_tryReadBinary()
{
	CFStream stream(_file.c_str(), L"rb");
	if (!stream.IsOpen())
		return false;

	int sign = 0;
	stream.Read(&sign, sizeof(int));

	if (sign != KEY_VALUE_SIGNITURE)
	{
		//stream.close();
		return false;
	}

	int len = (int)strnlen_s(KV_VERSION, 15);
	char* version = new char[len];
	stream.Read(version, len);
	if (strcmp(version, KV_VERSION) != 0)
	{
		//stream.close();
		return false;
	}

	_readBinary(stream);
	//stream.close();
	return true;
}

void KVCategory::_readBinary(CFStream& stream)
{
	stream >> Name;
	int numValues, numCategories;
	stream >> &numValues >> &numCategories;

	if (numValues != 0)
		Values.Reserve(numValues);

	for (int i = 0; i < numValues; i++)
	{
		FString valueName;
		FString value;
		stream >> valueName >> value;

		Values.Add({ valueName, KVValue(value) });
	}

	for (int i = 0; i < numCategories; i++)
	{
		//String valueName;
		//String value;
		//stream >> valueName >> value;

		KVCategory* nc = new KVCategory();
		nc->Parent = this;
		nc->_readBinary(stream);
	}
}

void KVCategory::_saveBinary(CFStream& stream) const
{
	stream << Name;
	int numValues = (int)Values.Size();
	int numCategories = (int)Categories.Size();
	stream << &numValues << &numCategories;

	for (auto& v : Values)
		stream << v.Key << v.Value.Value;

	for (auto& c : Categories)
		c->_saveBinary(stream);
}

struct FMacroToken
{
	enum EType
	{
		VARIABLE,
		NUMBER,
		OPERATOR
	};

	EType type;
	FString value;
};

bool GetMacroValue(FKeyValue* kv, TArray<FMacroToken>& tokens, int i)
{
	FMacroToken token = tokens[i];
	int a = 0;
	int b = 0;
	bool result = false;

	if (tokens[i - 1].type == FMacroToken::VARIABLE)
	{
		if (auto* m = kv->GetMacro(tokens[i - 1].value); m != nullptr)
			a = m->Value;
	}
	else if (tokens[i - 1].type == FMacroToken::NUMBER)
		a = tokens[i - 1].value.ToInt();
	if (tokens[i + 1].type == FMacroToken::VARIABLE)
	{
		if (auto* m = kv->GetMacro(tokens[i + 1].value); m != nullptr)
			b = m->Value;
	}
	else if (tokens[i + 1].type == FMacroToken::NUMBER)
		b = tokens[i + 1].value.ToInt();

	if (token.value == "<")
		result = a < b;
	else if (token.value == ">")
		result = a > b;
	else if (token.value == ">=")
		result = a >= b;
	else if (token.value == "<=")
		result = a <= b;
	else if (token.value == "==")
		result = a == b;
	else if (token.value == "!=")
		result = a != b;

	return result;
}

bool FKeyValue::_tryReadAscii()
{
	std::ifstream stream(_file.c_str());
	if (!stream.is_open())
		return false;

	std::string line;
	KVCategory* Cat = this;
	TArray<FString>* curArray = nullptr;
	TArray<FString> values;
	TArray<bool> macroValues;
	FString curValue;
	int lineCount = 0;
	bool bInQuotes = false;
	bool bPrevSpace = false;
	int8 indents = 0;
	uint8 curType = 0;
	while (std::getline(stream, line))
	{
		lineCount++;
		line += '\n';

		size_t commentp = line.rfind("//");
		if (commentp != FString::npos)
			line.erase(line.begin() + commentp, line.end());

		if (line.empty())
			continue;

		if (!Cat)
			break;

		if (SizeType i = line.find('#'); i != -1)
		{
			TArray<FString> args = FString(line).Split(" \t");

			if (args[0] == "#ifdef")
			{
				macroValues.Add(GetMacro(args[1]) != nullptr);
			}
			if (args[0] == "#ifndef")
			{
				macroValues.Add(GetMacro(args[1]) == nullptr);
			}
			else if (args[0] == "#if")
			{
				args.Erase(args.first());

				TArray<FMacroToken> tokens;

				for (auto it : args)
				{
					if (it == "<" || it == ">" || it == ">=" || it == "<="|| it == "==" || it == "!=" || it == "&&" || it == "||")
					{
						tokens.Add({ FMacroToken::OPERATOR, it });
						continue;
					}
					if (it.IsNumber())
					{
						tokens.Add({ FMacroToken::NUMBER, it });
						continue;
					}
					if (!it.IsEmpty())
						tokens.Add({ FMacroToken::VARIABLE, it });
				}

				bool bResult = false;

				for (int i = 0; i < tokens.Size(); i++)
				{
					if (tokens.Size() == 1)
					{
						if (tokens[i].type == FMacroToken::VARIABLE)
						{
							TPair<FString, int>* m = GetMacro(tokens[i].value);
							macroValues.Add(m ? m->Value : 0);
						}
						else if (tokens[i].type == FMacroToken::NUMBER)
						{
							macroValues.Add(tokens[i].value.ToInt());
						}
					}

					if (tokens[i].type == FMacroToken::OPERATOR)
					{
						if (tokens[i].value == "&&")
						{
							bResult = bResult && GetMacroValue(this, tokens, i + 2);
							i += 4;
						}
						else if (tokens[i].value == "||")
						{
							bResult = bResult || GetMacroValue(this, tokens, i + 2);
							i += 4;
						}
						else
							bResult = GetMacroValue(this, tokens, i);
					}
				}

				macroValues.Add(bResult);
			}
			else if (args[0] == "#endif")
			{
				if (macroValues.Size() == 0)
				{
					error = "invalid format: unexpected #endif at line " + FString::ToString(lineCount);
					return false;
				}
				macroValues.PopBack();
			}
		}

		if (macroValues.Size() > 0 && *macroValues.last() == false)
			continue;

		for (auto i = 0; i != line.size(); i++)
		{
			char ch = line[i];
			if (ch == '\n')
			{
				if (curType != 2)
				{
					if (!curValue.IsEmpty())
						values.Add(curValue);
					curValue.Clear();
				}

				if (curType == 1)
				{
					if (values.Size() == 2)
						Cat->Values.Add({ *values.last()--, KVValue(*values.last()) });
					else if (values.Size() == 1)
						Cat->Values.Add({ *values.last(), KVValue() });

					values.Clear();
					curType = 0;
				}
				continue;
			}

			if ((ch == '\t' || ch == ' ') && !bInQuotes)
			{
				if (!bPrevSpace && curType != 2)
				{
					if (!curValue.IsEmpty())
						values.Add(curValue);
					curValue.Clear();

					if (curType == 1 && values.Size() > 1)
					{
						Cat->Values.Add({ *values.last()--, KVValue(*values.last()) });
						values.Clear();
						curType = 0;
					}
				}

				bPrevSpace = true;
				continue;
			}

			if (ch == '"' || ch == '\'')
			{
				bInQuotes ^= 1;
				continue;
			}

			if (!bInQuotes)
			{
				if (ch == '{')
				{
					indents++;
					if (indents > 1)
					{
						KVCategory* c = new KVCategory();
						c->Parent = Cat;
						c->Name = values.Size() > 0 ? *values.last() : FString();
						Cat->Categories.Add(c);
						Cat = c;
					}
					values.Clear();
					continue;
				}
				else if (ch == '}')
				{
					indents--;
					values.Clear();
					curValue.Clear();
					Cat = Cat->Parent;
					continue;
				}
				else if (ch == '[')
				{
					curType = 2;
					curArray = Cat->GetArray(bPrevSpace ? *values.last() : curValue, true);
					values.Clear();
					continue;
				}
				else if (ch == ']')
				{
					curType = 0;
					values.Clear();
					if (!curValue.IsEmpty())
						curArray->Add(curValue);
					continue;;
				}
				else if (ch == ':')
				{
					if (indents == 0)
					{
						error = "invalid format: unexpected value outside of body at line " + FString::ToString(lineCount);
						return false;
					}

					values.Clear();
					curType = 1;
					continue;
				}

				if (curType == 2 && ch == ',')
				{
					curArray->Add(curValue);
					curValue.Clear();
					continue;
				}
			}

			curValue += ch;
			bPrevSpace = false;

			//if (i == line.size() - 1)
			//{
			//	if (curType == 1)
			//	{
			//		Cat->Values.Add({ *values.last(), KVValue(curValue) });
			//		values.Clear();
			//		curType = 0;
			//	}
			//}
		}
	}

	if (indents > 0)
	{
		error = "invalid format: expected '}' at line " + FString::ToString(lineCount);
		return false;
	}
	if (macros.Size() > 0)
	{
		error = "invalid format: expected #endif at line " + FString::ToString(lineCount);
		return false;
	}

	bIsOpen = true;
	stream.close();
	return true;
}

TPair<FString, int>* FKeyValue::GetMacro(const FString& name)
{
	for (auto& m : macros)
	{
		if (m.Key == name)
			return &m;
	}
	return nullptr;
}

void KVCategory::_save(std::ofstream& stream, int& tabs) const
{
	WRITE_TABS()
	FString kv1;
	if (Name.FindFirstOf(' ') != -1)
		kv1 = FString("\"") + Name + "\"\n{\n";
	else
		kv1 = Name + "\n";

	stream.write(kv1.Data(), kv1.Size());

	if (tabs != 0)
		WRITE_TABS()

	kv1 = "{\n";
	stream.write(kv1.Data(), kv1.Size());

	tabs++;
	for (auto& v : Values)
	{
		WRITE_TABS()
		FString vs;
		if (!v.Value.Value.IsNumber())
			vs = v.Key + ": \"" + v.Value.Value + "\"\n";
		else
			vs = v.Key + ": " + v.Value.Value + "\n";
		stream.write(vs.Data(), vs.Size());
	}

	if (Categories.Size() > 0)
	{
		kv1 = "\n";
		stream.write(kv1.Data(), kv1.Size());
	}

	for (auto& c : Categories)
		c->_save(stream, tabs);

	tabs--;

	WRITE_TABS()
	kv1 = "}\n";
	stream.write(kv1.Data(), kv1.Size());
}

#undef WRITE_TABS

float KVValue::AsFloat(float fallback) const
{
	if (Value.Size() == 0)
		return fallback;

	for (auto& ch : Value)
		if ((ch > 57 || ch < 48) && ch != 46)
			return fallback;

	return std::stof(Value.c_str());
}

int KVValue::AsInt(int fallback) const
{
	if (Value.Size() == 0)
		return fallback;

	for (auto& ch : Value)
		if (ch > 57 || ch < 48)
			return fallback;

	return std::stoi(Value.c_str());
}

bool KVValue::AsBool() const
{
	if (Value == "false")
		return false;
	else if (Value == "true")
		return true;

	return AsInt();
}
