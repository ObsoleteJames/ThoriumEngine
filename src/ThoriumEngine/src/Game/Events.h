#pragma once

#include "EngineCore.h"
#include "Object/Delegate.h"

class ENGINE_API Events
{
public:
	static TDelegate<> OnEngineInit;

	static TDelegate<> OnUpdate;
	static TDelegate<> PostUpdate;

	static TDelegate<> OnRender;
	static TDelegate<> PostRender;
	
	static TDelegate<> LevelChange;
	static TDelegate<> PostLevelChange;
};
