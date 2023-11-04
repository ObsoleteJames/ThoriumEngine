#pragma once

#include "Layer.h"
#include "EngineCore.h"

class CProjectSettingsWidget : public CLayer
{
public:
	void OnUIRender() override;

public:
	void RenderGameSettings();
	void RenderAudioSettings();
	void RenderInputSettings();
	void RenderPhysicsSettings();

private:
	int curMenu = 0;

};
