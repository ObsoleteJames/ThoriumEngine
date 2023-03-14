#pragma once

#include "Windows/ToolsWindow.h"
#include "Console.h"

class CWidgetDemo : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CWidgetDemo, "Widget Demo", false)

public:
	bool Shutdown() override { SaveState(); return true; }
	void SetupUi() override;

private:
	using TArr = TArray<uint>;
	TArr arr;
};
