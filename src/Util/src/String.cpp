
#include "Util/String.h"
#include <cstring>

SizeType StrLen(const char* str)
{
	return strlen(str);
}

SizeType StrLen(const wchar_t* str)
{
	return wcslen(str);
}

// WString ToWString(const FString& str)
// {
// 	WString r;
// 	r.Reserve(str.Size() + 1);
// 	//mbstowcs(r.Data(), str.Data(), str.Size());
// 	for (SizeType i = 0; i < str.Size(); i++)
// 		r += ((wchar_t)str[i]);

// 	return r;
// }

// FString ToFString(const WString& str)
// {
// 	FString r;
// 	r.Reserve(str.Size() + 1);
// 	for (SizeType i = 0; i < str.Size(); i++)
// 		r += ((char)str[i]);

// 	return r;
// }

// WString operator+(const wchar_t* a, const WString& b)
// {
// 	return WString(a) + b;
// }

FString operator+(const char* a, const FString& b)
{
	return FString(a) + b;
}
