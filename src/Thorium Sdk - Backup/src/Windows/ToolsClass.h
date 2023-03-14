#pragma once

#include "ToolsCore.h"
#include <QIcon>

class CToolsWindow;
class CToolsWidget;

struct FToolsClassBase
{
	const char* Name;
	const char* ToolBarPath;
	SizeType Id;
	QIcon* icon;
};

struct FToolsWindowClass : public FToolsClassBase
{
	virtual CToolsWindow* Create() = 0;
};

struct FToolsWidgetClass : public FToolsClassBase
{
	virtual CToolsWidget* Create(CToolsWindow* parent) = 0;

	SizeType WindowHash = 0;
	bool bShowOnStart;
};

//extern TArray<FToolsWindowClass*> _RegisteredWindows;
//extern TArray<FToolsWidgetClass*> _RegisteredWidgets;
