
#include "MgsDemo.h"
#include "Game/GameMode.h"
#include "MgsGameMode.generated.h"

CLASS()
class MGS_API CMgsGameMode : public CGameMode
{
	GENERATED_BODY()

public:
	void Init() override;

};
