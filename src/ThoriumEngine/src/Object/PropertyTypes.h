#pragma once

#include "EngineCore.h"

struct FArrayHelper
{
	void(*AddEmpty)(void* ptr);
	void(*Erase)(void* ptr, SizeType i);
	void(*Clear)(void* ptr);
	SizeType(*Size)(void* ptr);
	void*(*Data)(void* ptr);
	
	uint objType;
	SizeType objSize;
};
