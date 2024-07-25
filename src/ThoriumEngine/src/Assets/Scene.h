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

	//void Save(CWorld* world);
	
protected:
	void OnSave(IBaseFStream* stream);
	void OnLoad(IBaseFStream* stream, uint8 lodLevel) {}

	virtual uint8 GetFileVersion() const;

public:
	PROPERTY(Editable, Category = "Gamemode")
	TClassPtr<CGameMode> gamemodeClass = CGameMode::StaticClass();

	PROPERTY(Editable, Category = "Physics")
	float gravity = 9.81f;

private:
	CWorld* world;

};
