#pragma once

#include "EngineCore.h"

class IBaseWindow;

class ENGINE_API CInputManager
{
public:
	static void SetInputWindow(IBaseWindow* window);


private:
	static IBaseWindow* inputWindow;

};
