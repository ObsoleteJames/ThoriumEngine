
#include "Util/String.h"

SizeType StrLen(const char* str)
{
	return strlen(str);
}

SizeType StrLen(const wchar_t* str)
{
	return wcslen(str);
}

WString ToWString(const FString& str)
{
	WString r;
	r.Reserve(str.Size());
	//mbstowcs(r.Data(), str.Data(), str.Size());
	for (SizeType i = 0; i < str.Size(); i++)
		r[i] = (wchar_t)str[i];

	return r;
}

FString ToFString(const WString& str)
{
	FString r;
	r.Reserve(str.Size());
	for (SizeType i = 0; i < str.Size(); i++)
		r[i] = (char)str[i];

	return r;
}
