
#include "CppTypes.h"

CppFunction* CppClass::GetFunction(const FString& name)
{
	for (auto& f : Functions)
		if (f.name == name)
			return &f;

	return nullptr;
}

