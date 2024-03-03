#pragma once

#include "EngineCore.h"
#include "Game/Entity.h"
#include "MgsDemo.h"
#include "TestEntity.generated.h"

STRUCT()
struct MGS_API FTestStruct
{
	GENERATED_BODY()

public:
	PROPERTY(Editable)
	float tesFloat;

	PROPERTY(Editable)
	int tesInt;

};

CLASS()
class MGS_API CTestEntity : public CEntity
{
	GENERATED_BODY()

public:
	PROPERTY(Editable)
	int testInt;

	PROPERTY(Editable)
	FTestStruct tesStruct;
	
};
