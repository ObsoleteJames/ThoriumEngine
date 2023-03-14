
#include <fstream>
#include <string>
#include "Util/KeyValue.h"

// 2 byte value
#define KEY_VALUE_SIGNITURE 0x36D6
const char* KV_VERSION = "KV_BINARY_1.00";

KVCategory::~KVCategory()
{
	for (auto& c : Categories)
		delete c.Value;

	for (auto& v : Values)
		delete v.Value;

	Categories.Clear();
	Values.Clear();
}

KVCategory* KVCategory::GetCategory(const FString& name, bool cie)
{
	auto it = Categories.Find([=](const TPair<FString, KVCategory*>& i) { return i.Key == name; });
	if (it != Categories.end())
		return it->Value;

	if (cie)
	{
		KVCategory* c = new KVCategory();
		c->Parent = this;
		c->Name = name;
		Categories.Add({ name, c });
		return c;
	}

	return nullptr;
}

KVValue* KVCategory::GetValue(const FString& key, bool cie)
{
	for (auto& t : Values)
		if (t.Key == key)
			return t.Value;

	if (cie)
	{
		Values.Add({ key, new KVValue() });
		return Values.At(Values.Size() - 1)->Value;
	}
	return nullptr;
}

FKeyValue::FKeyValue(const FString& file, const EKeyValueType& type)
{
	Open(file);
}

FKeyValue::~FKeyValue()
{
	for (auto& c : Categories)
		delete c.Value;

	for (auto& v : Values)
		delete v.Value;

	Categories.Clear();
	Values.Clear();
}

bool FKeyValue::Open(const FString& file, const EKeyValueType& type)
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

bool FKeyValue::Save(const FString& file, const EKeyValueType& type) const
{
	if (type == KV_BINARY)
	{
		CFStream stream(file.c_str(), "wb");

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

bool FKeyValue::_tryReadBinary()
{
	CFStream stream(_file.c_str(), "rb");
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

		Values.Add({ valueName, new KVValue(value) });
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
		stream << v.Key << v.Value->Value;

	for (auto& c : Categories)
		c.Value->_saveBinary(stream);
}

bool FKeyValue::_tryReadAscii()
{
	std::ifstream stream(_file.c_str());
	if (!stream.is_open())
		return false;

	bIsOpen = true;

	std::string line;
	KVCategory* Cat = this;
	int indents = 0;
	while (std::getline(stream, line))
	{
		if (!Cat)
			break;

		size_t commentp = line.rfind("//");
		if (commentp != FString::npos)
			line.erase(line.begin() + commentp, line.end());

		if (line.empty())
			continue;

		if (line.find('{') != FString::npos)
		{
			//Cat = (Cat->Categories.end()--)->second;
			indents++;
			continue;
		}
		if (line.find('}') != FString::npos)
		{
			Cat = Cat->Parent;
			indents--;
			continue;
		}

		TArray<FString> LStrings;
		int VarAmount = 0;
		bool prevWasSpace = false;
		bool bStartNew = true;
		bool bInQoute = false;
		for (auto& ch : line)
		{
			if ((ch == ' ' || ch == '	') && !bInQoute)
			{
				prevWasSpace = true;
				bStartNew = false;
				continue;
			}

			if (bStartNew || prevWasSpace)
			{
				if (LStrings.Size() == 2)
					break;

				LStrings.Add(FString());
			}

			if (ch == '"')
			{
				bInQoute = !bInQoute;
				prevWasSpace = false;
				continue;
			}

			LStrings[LStrings.Size() - 1].Add(ch);
			prevWasSpace = false;
			bStartNew = false;
		}

		if (LStrings.Size() == 2)
		{
			Cat->Values.Add({ LStrings[0], new KVValue(LStrings[1]) });
			continue;
		}
		if (LStrings.Size() == 1)
		{
			if (indents == 0)
				Cat->Name = LStrings[0];
			else
			{
				KVCategory* nc;
				{
					auto it = Cat->Categories.Find([=](const TPair<FString, KVCategory*>& i) { return i.Key == LStrings[0]; });
					if (it == Cat->Categories.end())
					{
						nc = new KVCategory();
						Cat->Categories.Add({ LStrings[0], nc });
					}
					else
						nc = it->Value;
				}

				nc->Parent = Cat;
				nc->Name = LStrings[0];
				Cat = nc;
				continue;
			}
		}

		LStrings.Clear();
	}

	stream.close();
	return true;
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
		FString vs = FString("\"") + v.Key + "\"\t\"" + v.Value->Value + "\"\n";
		stream.write(vs.Data(), vs.Size());
	}

	if (Categories.Size() > 0)
	{
		kv1 = "\n";
		stream.write(kv1.Data(), kv1.Size());
	}

	for (auto& c : Categories)
		c.Value->_save(stream, tabs);

	tabs--;

	WRITE_TABS()
	kv1 = "}\n";
	stream.write(kv1.Data(), kv1.Size());
}

#undef WRITE_TABS

float KVValue::AsFloat() const
{
	if (Value.Size() == 0)
		return 0.0f;

	for (auto& ch : Value)
		if ((ch > 57 || ch < 48) && ch != 46)
			return 0.0f;

	return std::stof(Value.c_str());
}

int KVValue::AsInt() const
{
	if (Value.Size() == 0)
		return 0;

	for (auto& ch : Value)
		if (ch > 57 || ch < 48)
			return 0;

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
