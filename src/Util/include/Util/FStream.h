#pragma once

#include <Util/Types.h>
#include <Util/String.h>
#include <Util/Pointer.h>

// #ifndef _FILE_DEFINED
// #define _FILE_DEFINED
// typedef struct _iobuf
// {
// 	void* _Placeholder;
// } FILE;
// #endif

// These should only be used for binary data.
class IBaseFStream
{
public:
	virtual ~IBaseFStream() = default;

	template<typename T>
	inline IBaseFStream& operator<<(T* buff)
	{
		Write(buff, sizeof(T));
		return *this;
	}
	template<typename T>
	inline IBaseFStream& operator>>(T* buff)
	{
		Read(buff, sizeof(T));
		return *this;
	}

	inline IBaseFStream& operator<<(const char* str)
	{
		Write((void*)str, StrLen(str) + 1);
		return *this;
	}

	template<typename T>
	inline IBaseFStream& operator<<(const TBaseString<T>& str)
	{
		Write((void*)str.Data(), (str.Size() + 1) * sizeof(T));
		return *this;
	}

	template<typename T>
	IBaseFStream& operator>>(TBaseString<T>& str)
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

	virtual void Read(void* buff, size_t size) = 0;
	virtual void Write(void* buff, size_t size) = 0;
	virtual void Seek(size_t pos, int8 from) = 0;
	inline size_t Tell() { return offset; }
	inline bool IsOpen() { return bIsOpen; }

protected:
	bool bIsOpen = false;
	size_t offset;
};

class CFStream : public IBaseFStream
{
public:
	CFStream() = default;
	CFStream(const FString& file, const char* mode);
	//CFStream(const WString& file, const wchar_t* mode);
	virtual ~CFStream();

	void Open(const FString& file, const char* mode);
	void Close();

	void Read(void* buff, size_t size) override;
	void Write(void* buff, size_t size) override;
	void Seek(size_t pos, int8 from) override;
	
private:
	FILE* f;
};
