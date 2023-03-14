#pragma once

#include "Asset.h"
#include "Game/GameMode.h"
#include "Scene.generated.h"

class CWorld;

#define CSCENE_VERSION 0x0001
#define CSCENE_SIGNITURE 0xD3AFB1C8

ASSET(Extension = ".thscene")
class ENGINE_API CScene : public CAsset
{
	GENERATED_BODY()

	friend class CWorld;

public:
	CScene() = default;

	virtual void Init() {}

	void Save(CWorld* world);
	virtual void Load(uint8) {}

protected:
	PROPERTY(Editable)
	TClassPtr<CGameMode> gamemodeClass = CGameMode::StaticClass();

};
