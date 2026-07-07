
#include "Events.h"

TDelegate<> Events::OnEngineInit;
TDelegate<> Events::OnEnginePostInit;
TDelegate<> Events::OnUpdate;
TDelegate<> Events::PostUpdate;
TDelegate<> Events::OnRender;
TDelegate<> Events::PostRender;
TDelegate<> Events::LevelChange;
TDelegate<> Events::PostLevelChange;
