#pragma once

#include "ToolsWindow.h"
#include <Util/String.h>

class CProjectManagerWnd : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CProjectManagerWnd, "Project Manager", false)

public:
	CProjectManagerWnd();
	virtual ~CProjectManagerWnd();

protected:
	virtual bool Shutdown() override;
	virtual void SetupUi() override;

};
