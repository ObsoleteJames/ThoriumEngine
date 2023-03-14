#pragma once

#include "Core.h"
#include "FStream.h"

struct KVValue
{
	FString Value;

	KVValue() = default;
	KVValue(const char* v) : Value(FString(v)) {}
	KVValue(const FString& v) : Value(v) {}

	operator FString&() { return Value; }
	operator const FString&() const { return Value; }

	float AsFloat() const;
	int AsInt() const;
	bool AsBool() const;
};

struct KVCategory
{
	friend class FKeyValue;

public:
	~KVCategory();
	KVCategory* GetCategory(const FString& name, bool CreateIfEmpty = false);
	KVValue* GetValue(const FString& key, bool CreateIfEmpty = true);

	inline const FString& GetName() const { return Name; }
	inline const TArray<TPair<FString, KVValue*>>& GetValues() const { return Values; }
	inline const TArray<TPair<FString, KVCategory*>>& GetCategories() const { return Categories; }

protected:
	void _save(std::ofstream& stream, int& tabs) const;
	void _saveBinary(CFStream& stream) const;
	void _readBinary(CFStream& stream);

protected:
	KVCategory* Parent = nullptr;
	FString Name;
	TArray<TPair<FString, KVValue*>> Values;
	TArray<TPair<FString, KVCategory*>> Categories;
};

enum EKeyValueType
{
	KV_UNKOWN = 0,
	KV_STANDARD_ASCII,
	KV_BINARY
};

class FKeyValue : public KVCategory
{
public:
	FKeyValue() = default;
	FKeyValue(const FString& file, const EKeyValueType& type = KV_UNKOWN);
	~FKeyValue();
	
	bool Open(const FString& file, const EKeyValueType& type = KV_UNKOWN);
	bool Save(const FString& file, const EKeyValueType& type = KV_STANDARD_ASCII) const;
	bool Save(const EKeyValueType& type = KV_STANDARD_ASCII) const;

private:
	bool _tryReadBinary();
	bool _tryReadAscii();

public:
	bool bIsOpen = false;

private:
	FString _file;

};
