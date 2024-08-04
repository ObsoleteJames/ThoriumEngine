#pragma once

#include "EngineCore.h"
#include "Class.h"

class FArrayHelper
{
public:
	FArgType objType;
	SizeType objSize;

public:
	virtual void AddEmpty(void* ptr) = 0;
	virtual void Erase(void* ptr, SizeType index) = 0;
	virtual void Clear(void* ptr) = 0;
	virtual SizeType Size(void* ptr) = 0;
	virtual void* Data(void* ptr) = 0;
};
