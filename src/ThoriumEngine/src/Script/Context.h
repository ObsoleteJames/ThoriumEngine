#pragma once

#include "Frame.h"

class CScriptContext
{
public:
	CScriptContext() = default;

public:
	FVariable returnValue;
	uint8 lastOperatoinValue;

	void* object; // this

	FStack stack;
	TArray<FArgType> stackTypes;

	TArray<FVariable> localVars;
};
