#pragma once

#include "Util/Array.h"

SizeType StrLen(const char*);
SizeType StrLen(const wchar_t*);

// use std::string or custom string.
#if 0
#include <string>
using FString = std::string;
using WString = std::wstring;
#else
template<typename T>
class TBaseString : public TArray<T>
{
	static_assert(std::is_integral<T>::value, "TBaseString does not support non integral types.");

public:
	TBaseString();
	TBaseString(const T*);
	TBaseString(const TBaseString<T>&);

	TBaseString<T>& operator=(const T*);
	TBaseString<T>& operator=(const TBaseString<T>&);

	template<typename T2>
	static TBaseString<T> ToString(T2);

#ifdef _STRING_ // std::string support
	inline TBaseString<T>& operator=(const std::basic_string<T>& str) { return (*this = str.c_str()); }
	inline TBaseString(const std::basic_string<T>& str) { *this = str.c_str(); }

	inline TBaseString<T> operator+(const std::basic_string<T>& str) { return (*this + str.c_str()); }
	inline TBaseString<T> operator+=(const std::basic_string<T>& str) { return (*this += str.c_str()); }
#endif

	inline TBaseString<T> operator+(T ch) const { return TBaseString<T>(*this) += ch; }
	inline TBaseString<T> operator+(const T* str) const { return TBaseString<T>(*this) += str; }
	inline TBaseString<T> operator+(const TBaseString<T>& str) const { return TBaseString<T>(*this) += str; }

	inline TBaseString<T> operator+(T ch) { return TBaseString<T>(*this) += ch; }
	inline TBaseString<T> operator+(const T* str) { return TBaseString<T>(*this) += str; }
	inline TBaseString<T> operator+(const TBaseString<T>& str) { return TBaseString<T>(*this) += str; }

	inline TBaseString<T>& operator+=(const TBaseString<T>&);
	inline TBaseString<T>& operator+=(const T*);
	inline TBaseString<T>& operator+=(T ch);

	inline bool operator==(const TBaseString<T>&) const;
	inline bool operator==(const T*) const;

	inline bool operator!=(const TBaseString<T>&) const;
	inline bool operator!=(const T*) const;

	void Resize(SizeType size);

	void Erase(const TIterator<T>& index);
	void Erase(const TIterator<T>& first, const TIterator<T>& last);

	void Insert(const T&, SizeType index);

	SizeType GetLine(TBaseString<T>& out, SizeType offset = 0) const;

	inline const T* c_str() const;
	SizeType Hash() const;

	inline void Clear();

	void Reverse();
	inline bool IsEmpty(bool search = true) const;
	inline bool IsNumber() const;

	int ToInt();

	void EraseAll(char ch, SizeType offset = 0);
	void ReplaceAll(char from, char to, SizeType offset = 0);

	TArray<TBaseString<T>> Split(T ch, SizeType offset = 0) const;
	TArray<TBaseString<T>> Split(const TBaseString<T>& str, SizeType offset = 0) const;

	SizeType Find(const T* str, SizeType offset = 0) const;
	SizeType Find(const TBaseString<T>& str, SizeType offset = 0) const;

	SizeType FindFirstOf(T ch, SizeType offset = 0) const;
	SizeType FindFirstOf(const TBaseString<T>& str, SizeType offset = 0) const;
	SizeType FindLastOf(T ch, SizeType offset = 0) const;
	SizeType FindLastOf(const TBaseString<T>& str, SizeType offset = 0) const;

public:
	static SizeType npos;

};

template<typename T>
void TBaseString<T>::Resize(SizeType size)
{
	if (this->_size >= size)
		return;

	SizeType prevSize = this->_size;

	this->_size = size;
	this->Reserve(this->_size + 2);

	this->_data[this->_size + 1] = (T)L'\0';
}

template<typename T>
SizeType TBaseString<T>::GetLine(TBaseString<T>& out, SizeType offset /*= 0*/) const
{
	out.Clear();
	for (auto ptr = offset; ptr < this->_size; ptr++)
	{
		if (this->_data[ptr] == (T)L'\n')
			return ptr + 1;

		out += (T)this->_data[ptr];
	}
	return -1;
}

template<typename T>
void TBaseString<T>::Insert(const T& obj, SizeType index)
{
	if (index >= this->_size)
		return;

	SizeType amount = this->_size - index;
	this->Reserve(this->_size + 2);

	if (amount != 0)
		memcpy(&this->_data[index + 1], &this->_data[index], amount * sizeof(T));

	if constexpr (!std::is_pointer<T>::value && std::is_copy_constructible<T>::value)
		new (&this->_data[index]) T(obj);
	else
		this->_data[index] = obj;

	this->_size++;
	this->_data[this->_size] = '\0';
}

template<typename T>
template<typename T2>
TBaseString<T> TBaseString<T>::ToString(T2 i)
{
	TBaseString<T> r;
	bool bNegative = false;

	if (i < 0)
	{
		bNegative = true;
		i *= -1;
	}

	do
	{
		int v = i % 10;
		r += (T)v + (T)L'0';
		i /= 10;
	}
	while (i != 0);

	if (bNegative)
		r += (T)L'-';
	r.Reverse();
	return r;
}

#pragma region DEFINITION

template<typename T>
SizeType TBaseString<T>::npos = -1;

template<typename T>
TBaseString<T>::TBaseString() : TArray<T>()
{
	// This causes an exception since the constructor of TArray already allocates memory
	//TArray<T>::_data = nullptr;
	//TArray<T>::_capacity = 0;
	//TArray<T>::_size = 0;
	//TArray<T>::Reserve(2, false);
	TArray<T>::_data[0] = '\0';
}

template<typename T>
TBaseString<T>::TBaseString(const T* str) : TArray<T>()
{
	//this->_data = nullptr;
	SizeType newSize = StrLen(str);
	this->Reserve(newSize + 1);
	this->_size = newSize;

	memcpy(this->_data, str, this->_size * sizeof(T));
	this->_data[this->_size] = '\0';
}

template<typename T>
TBaseString<T>::TBaseString(const TBaseString<T>& str) : TArray<T>()
{
	//this->_data = nullptr;
	this->Reserve(str.Size() + 1, false);
	this->_size = str.Size();

	memcpy(this->_data, str._data, this->_size * sizeof(T));
	this->_data[this->_size] = '\0';
}

template<typename T>
TBaseString<T>& TBaseString<T>::operator=(const T* str)
{
	SizeType newSize = StrLen(str);
	this->Reserve(newSize + 1, false);
	this->_size = newSize;

	memcpy(this->_data, str, this->_size * sizeof(T));
	this->_data[this->_size] = '\0';
	return *this;
}

template<typename T>
TBaseString<T>& TBaseString<T>::operator=(const TBaseString<T>& str)
{
	this->Reserve(str.Size() + 1, false);
	this->_size = str.Size();

	memcpy(this->_data, str._data, this->_size * sizeof(T));
	this->_data[this->_size] = '\0';
	return *this;
}

template<typename T>
TBaseString<T>& TBaseString<T>::operator+=(const TBaseString<T>& str)
{
	this->Reserve(this->_size + str._size + 1);
	SizeType oldSize = this->_size;
	this->_size += str._size;

	memcpy(this->_data + oldSize, str._data, str._size * sizeof(T));
	this->_data[this->_size] = '\0';
	return *this;
}

template<typename T>
TBaseString<T>& TBaseString<T>::operator+=(const T* str)
{
	SizeType oldSize = this->_size;
	SizeType strLen = StrLen(str);

	this->Reserve(this->_size + strLen + 1);
	this->_size += strLen;

	memcpy(this->_data + oldSize, str, strLen * sizeof(T));
	this->_data[this->_size] = '\0';
	return *this;
}

template<typename T>
TBaseString<T>& TBaseString<T>::operator+=(T ch)
{
	this->Reserve(this->_size + 2);
	this->_size++;

	this->_data[this->_size] = '\0';
	this->_data[this->_size - 1] = ch;
	return *this;
}

template<typename T>
bool TBaseString<T>::operator==(const TBaseString<T>& str) const
{
	if (str.Size() != this->_size)
		return false;

	for (SizeType i = 0; i < this->_size; i++)
		if (str[i] != this->_data[i])
			return false;

	return true;
}

template<typename T>
bool TBaseString<T>::operator==(const T* str) const
{
	for (SizeType i = 0; i < this->_size + 1; i++)
	{
		if (!str[i] && this->_data[i])
			return false;

		if (str[i] != this->_data[i])
			return false;
	}

	return true;
}

template<typename T>
bool TBaseString<T>::operator!=(const TBaseString<T>& str) const
{
	return !(*this == str);
}

template<typename T>
bool TBaseString<T>::operator!=(const T* str) const
{
	return !(*this == str);
}

template<typename T>
void TBaseString<T>::Erase(const TIterator<T>& index)
{
	if (index == TArray<T>::end())
		throw std::out_of_range("");

	SizeType _i = ((SizeType)index.ptr - (SizeType)this->_data) / sizeof(T);
	SizeType amount = this->_size - _i - 1;
	if (amount != 0)
		memcpy(index.ptr, index.ptr + 1, amount * sizeof(T));

	this->_size--;
	this->_data[this->_size] = '\0';
}

template<typename T>
void TBaseString<T>::Erase(const TIterator<T>& first, const TIterator<T>& last)
{
	if (last == TArray<T>::end() && first == TArray<T>::begin())
		return TBaseString<T>::Clear();

	if (last > TArray<T>::end() || first >= last)
		throw std::out_of_range("TBaseString<T>::Erase index out of range");

	SizeType removeAmount = ((SizeType)last.ptr - (SizeType)first.ptr) / sizeof(T);
	//SizeType findex = ((SizeType)first - (SizeType)data) / sizeof(T);
	SizeType lindex = ((SizeType)last.ptr - (SizeType)this->_data) / sizeof(T);

	if (std::is_class<T>::value && std::is_destructible<T>::value)
		for (TIterator<T> it = first; it != last; it++)
			(*it).~T();

	if (lindex != this->_size)
		memcpy(first.ptr, last.ptr, (this->_size - lindex) * sizeof(T));

	this->_size -= removeAmount;
	this->_data[this->_size] = '\0';
}

template<typename T>
const T* TBaseString<T>::c_str() const
{
	return this->_data;
}

template<typename T>
SizeType TBaseString<T>::Hash() const
{
	SizeType _hash = 5381;
	int c;

	char* ptr = this->_data;
	while (c = *ptr++)
		_hash = ((_hash << 5) + _hash) + c;

	return _hash;
}

template<typename T>
void TBaseString<T>::Clear()
{
	this->_data[0] = '\0';
	this->_size = 0;
}

template<typename T>
void TBaseString<T>::Reverse()
{
	SizeType halfSize = this->_size / 2;
	for (SizeType i = 0; i < halfSize; i++)
	{
		T backup = this->_data[i];
		SizeType opposite = (this->_size - i) - 1;
		this->_data[i] = this->_data[opposite];
		this->_data[opposite] = backup;
	}
}

template<typename T>
bool TBaseString<T>::IsEmpty(bool search /*= true*/) const
{
	if (this->_size == 0)
		return true;

	if (search)
	{
		for (int i = 0; i < this->_size; i++)
		{
			T ch = this->_data[i];
			if (ch != ' ' || ch != '	')
				return false;
		}
	}

	return search;
}

template<typename T>
bool TBaseString<T>::IsNumber() const
{
	for (auto& ch : *this)
	{
		if (ch != '.' && (ch < '0' || ch > '9'))
			return false;
	}
	return true;
}

template<typename T>
int TBaseString<T>::ToInt()
{
	int r = 0;

	bool bIsNegative = this->Data()[0] == '-';

	for (auto it = this->rbegin(); it != this->rend(); it++)
	{
		if (*it == '-')
			continue;

		T number = (*it) - '0';
		if (number > 9 || number < 0)
			throw std::exception();

		int i = ((it.ptr - this->Data()) / sizeof(T)) - bIsNegative;

		r += number * i;
	}

	if (bIsNegative)
		r = -r;

	return r;
}

template<typename T>
void TBaseString<T>::ReplaceAll(char from, char to, SizeType offset /*= 0*/)
{
	for (TIterator<T> it = TArray<T>::begin() + offset; it != TArray<T>::end(); it++)
	{
		if (*it == from)
			*it = to;
	}
}

template<typename T>
void TBaseString<T>::EraseAll(char ch, SizeType offset /*= 0*/)
{
	for (TReverseIterator<T> it = TArray<T>::rbegin() + offset; it != TArray<T>::rend(); it++)
	{
		if (*it == ch)
			this->Erase(it);
	}
}

template<typename T>
TArray<TBaseString<T>> TBaseString<T>::Split(T ch, SizeType offset /*= 0*/) const
{
	TArray<TBaseString<T>> arr;
	arr.Resize(1);
	for (auto& _c : *this)
	{
		if (_c == ch)
		{
			arr.Add(TBaseString<T>());
			continue;
		}

		arr[arr.Size() - 1] += _c;
	}

	return arr;
}

template<typename T>
TArray<TBaseString<T>> TBaseString<T>::Split(const TBaseString<T>& str, SizeType offset /*= 0*/) const
{
	TArray<TBaseString<T>> arr;
	arr.Resize(1);
	for (auto& _c : *this)
	{
		bool bContinue = false;
		for (auto& cmp : str)
		{
			if (_c == cmp)
			{
				arr.Add(TBaseString<T>());
				bContinue = true;
				break;
			}
		}

		if (!bContinue)
			arr[arr.Size() - 1] += _c;
	}

	return arr;
}

template<typename T>
SizeType TBaseString<T>::Find(const T* str, SizeType offset /*= 0*/) const
{
	SizeType str_size = StrLen(str);
	SizeType to_read = str_size;

	if (str_size > this->_size)
		return npos;

	SizeType i;
	for (i = offset; i < this->_size; i++)
	{
		if (to_read == 0)
			break;

		if (i + to_read > this->_size)
			return npos;

		if (this->_data[i] == str[str_size - to_read])
			to_read--;
		else
			to_read = str_size;
	}

	return to_read == 0 ? i - str_size : npos;
}

template<typename T>
SizeType TBaseString<T>::Find(const TBaseString<T>& str, SizeType offset /*= 0*/) const
{
	SizeType str_size = str.Size();
	SizeType to_read = str_size;

	if (str_size > this->_size)
		return npos;

	SizeType i;
	for (i = offset; i < this->_size; i++)
	{
		if (to_read == 0)
			break;

		if (i + to_read > this->_size)
			return npos;

		if (this->_data[i] == str[str_size - to_read])
			to_read--;
		else
			to_read = str_size;
	}

	return to_read == 0 ? i - str_size : npos;
}

template<typename T>
SizeType TBaseString<T>::FindFirstOf(T ch, SizeType offset /*= 0*/) const
{
	for (SizeType i = offset; i < this->_size; i++)
	{
		if (this->_data[i] == ch)
			return i;
	}
	return npos;
}

template<typename T>
SizeType TBaseString<T>::FindFirstOf(const TBaseString<T>& str, SizeType offset /*= 0*/) const
{
	for (SizeType i = offset; i < this->_size; i++)
	{
		for (SizeType c = 0; c < str.Size(); c++)
			if (this->_data[i] == str[c])
				return i;
	}
	return npos;
}

template<typename T>
SizeType TBaseString<T>::FindLastOf(T ch, SizeType offset /*= 0*/) const
{
	for (SizeType i = this->_size - offset; i > 0; i--)
	{
		if (this->_data[i - 1] == ch)
			return i - 1;
	}

	return npos;
}

template<typename T>
SizeType TBaseString<T>::FindLastOf(const TBaseString<T>& str, SizeType offset /*= 0*/) const
{
	for (SizeType i = this->_size - offset; i > 0; i--)
	{
		for (SizeType c = 0; c < str.Size(); c++)
			if (this->_data[i - 1] == str[c])
				return i - 1;
	}

	return npos;
}

#pragma endregion

namespace std
{
	template<>
	struct hash<TBaseString<char>>
	{
		size_t operator()(const TBaseString<char>& str) const
		{
			return hash<std::string>()(str.c_str());
		}
	};

	template<>
	struct hash<TBaseString<wchar_t>>
	{
		size_t operator()(const TBaseString<wchar_t>& str) const
		{
			return hash<std::wstring>()(str.c_str());
		}
	};
}

using FString = TBaseString<char>;
using WString = TBaseString<wchar_t>;

WString ToWString(const FString&);
FString ToFString(const WString&);

FString operator+(const char* a, const FString& b);
WString operator+(const wchar_t* a, const WString& b);

#endif
