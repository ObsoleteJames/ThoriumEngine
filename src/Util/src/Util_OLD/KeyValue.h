#pragma once

#include "Core.h"
#include "Map.h"
#include "FStream.h"

struct KVValue
{
	FString Value;

	KVValue() = default;
	KVValue(const char* v) : Value(FString(v)) {}
	KVValue(const FString& v) : Value(v) {}

	operator FString&() { return Value; }
	operator const FString&() const { return Value; }

	inline void Set(const FString& value) { Value = value; }

	float AsFloat(float fallback = 0.f) const;
	int AsInt(int fallback = 0) const;
	bool AsBool() const;
};

struct KVCategory
{
	friend class FKeyValue;

	typedef TArray<TPair<FString, TArray<FString>>> ArrayList;
	typedef TArray<TPair<FString, KVValue>> ValueList;
	typedef TArray<KVCategory*> CategoryList;

public:
	~KVCategory();
	KVCategory* GetCategory(const FString& name, bool CreateIfEmpty = false);
	KVValue* GetValue(const FString& key, bool CreateIfEmpty = true);
	inline void SetValue(const FString& key, const FString& value) { GetValue(key)->Set(value); }

	inline const FString& GetName() const { return Name; }
	inline const ValueList& GetValues() const { return Values; }
	inline const CategoryList& GetCategories() const { return Categories; }

	TArray<FString>* GetArray(const FString& name, bool CreateIfEmpty = false);
	inline const ArrayList& GetArrays() const { return arrays; }

protected:
	void _save(std::ofstream& stream, int& tabs) const;
	void _saveBinary(CFStream& stream) const;
	void _readBinary(CFStream& stream);

protected:
	KVCategory* Parent = nullptr;
	FString Name;

	ArrayList arrays;
	ValueList Values;
	CategoryList Categories;
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
	
	bool Open(const FString& file, const EKeyValueType& type = KV_UNKOWN);
	bool Save(const FString& file, const EKeyValueType& type = KV_STANDARD_ASCII) const;
	bool Save(const EKeyValueType& type = KV_STANDARD_ASCII) const;

	inline bool IsOpen() const { return bIsOpen; }

	inline const FString& GetLastError() const { return error; }
	
	/**
	 *	Defines a macro.
	 *	Usefull if the file contains platform specific data.
	 * 
	 *	NOTE: calling this will mark the file as read only!
	 */
	void DefineMacro(const FString& macro, int value);

	TPair<FString, int>* GetMacro(const FString& name);

private:
	bool _tryReadBinary();
	bool _tryReadAscii();

private:
	bool bIsOpen = false;
	bool bReadOnly = false;

private:
	TArray<TPair<FString, int>> macros; // Name, Value
	FString error;
	FString _file;

};
