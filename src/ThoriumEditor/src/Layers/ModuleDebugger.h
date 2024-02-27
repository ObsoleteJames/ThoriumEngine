#pragma once

#include "Layer.h"
#include "EditorCore.h"

class CModule;

class CModuleDebugger : public CLayer
{
public:
	void OnUIRender() override;

private:
	CModule* selected;
};
