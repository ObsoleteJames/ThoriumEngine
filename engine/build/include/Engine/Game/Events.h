#pragma once

#include "EngineCore.h"
#include "Object/Delegate.h"

class ENGINE_API Events
{
public:
	static TDelegate<> OnUpdate;
	static TDelegate<> PostUpdate;

	static TDelegate<> OnRender;
	
	static TDelegate<> LevelChange;
	static TDelegate<> PostLevelChange;
};
