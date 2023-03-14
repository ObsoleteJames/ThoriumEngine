#pragma once

#include "ToolsWindow.h"

class CEditorPreferences : public CToolsWindow
{
	Q_OBJECT

public:
	CEditorPreferences() = default;

	virtual void SetupUi() override;
	virtual bool Shutdown() override;

private:


};
