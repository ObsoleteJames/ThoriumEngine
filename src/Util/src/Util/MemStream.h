#pragma once

#include "Array.h"
#include "String.h"

class FMemStream : public TArray<uint8_t>
{
public:
	FMemStream() = default;
	inline FMemStream(const FMemStream& other) : TArray<uint8_t>(other), cursor(other.cursor) {}

	template<typename T>
	inline FMemStream& operator<<(T* buff)
	{
		Write((void*)buff, sizeof(T));
		return *this;
	}
	template<typename T>
	inline FMemStream& operator>>(T* buff)
	{
		Read((void*)buff, sizeof(T));
		return *this;
	}

	template<typename T>
	inline FMemStream& operator<<(const TBaseString<T>& str)
	{
		Write((void*)str.Data(), (str.Size() + 1) * sizeof(T));
		return *this;
	}

	template<typename T>
	FMemStream& operator>>(TBaseString<T>& str)
	{
		for (T ch;;)
		{
			Read(&ch, sizeof(T));
			if (ch == (T)L'\0')
				break;
			str += ch;
		}
		return *this;
	}

	inline FMemStream& operator<<(const FMemStream& other)
	{
		Write((void*)other.Data(), other.Size());
		return *this;
	}

	void Read(void* buffer, SizeType size);
	void Write(void* buffer, SizeType size);

	void Seek(SizeType to, uint8_t from);
	inline SizeType Tell() const { return cursor; }

	inline bool EndOfFile() { return cursor == Size(); }

private:
	SizeType cursor;
};
