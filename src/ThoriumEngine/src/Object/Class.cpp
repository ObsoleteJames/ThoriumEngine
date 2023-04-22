
#include "Class.h"

int64_t FEnum::GetValueByName(const FString& name)
{
	for (auto& v : values)
	{
		if (v.Key == name)
			return v.Value;
	}
	return -1;
}
