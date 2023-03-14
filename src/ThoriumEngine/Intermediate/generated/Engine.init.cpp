
#include "Module.h"

CModule& GetModule_Engine()
{
	static CModule _module("Engine");
	return _module;
}
