
#include <stdio.h>
#include "Util/FStream.h"

CFStream::CFStream(const FString& file, const char* mode)
{
	fopen_s(&f, file.c_str(), mode);
	bIsOpen = (f != nullptr);
}

CFStream::CFStream(const WString& file, const wchar_t* mode)
{
	_wfopen_s(&f, file.c_str(), mode);
	bIsOpen = (f != nullptr);
}

CFStream::~CFStream()
{
	if (f)
		fclose(f);
}

void CFStream::Open(const FString& file, const char* mode)
{
	Close();
	fopen_s(&f, file.c_str(), mode);
	bIsOpen = (f != nullptr);
}

void CFStream::Close()
{
	if (f)
		fclose(f);

	f = nullptr;
}

void CFStream::Read(void* buff, size_t size)
{
	fread(buff, size, 1, f);
	offset = ftell(f);
}

void CFStream::Write(void* buff, size_t size)
{
	fwrite(buff, size, 1, f);
	offset = ftell(f);
}

void CFStream::Seek(size_t pos, int8 from)
{
	fseek(f, pos, from);
	offset = ftell(f);
}
