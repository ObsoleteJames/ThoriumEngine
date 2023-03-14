#pragma once

#include "EngineCore.h"
#include <Util/Event.h>

class ENGINE_API Events
{
public:
	static TEvent<> OnUpdate;
	static TEvent<> PostUpdate;

	static TEvent<> OnRender;
	
	static TEvent<> LevelChange;
	static TEvent<> PostLevelChange;
};
