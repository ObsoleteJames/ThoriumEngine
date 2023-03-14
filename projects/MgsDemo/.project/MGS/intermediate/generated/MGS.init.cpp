
#include "Module.h"

CModule& GetModule_MGS()
{
	static CModule _module("MGS");
	return _module;
}

extern "C" __declspec(dllexport) CModule* __GetModuleInstance() { return &GetModule_MGS(); }
