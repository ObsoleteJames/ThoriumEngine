#pragma once

#include "Windows/ToolsWindow.h"

class QTreeWidget;
class QSplitter;
class QWidget;
class QScrollArea;
class CTreeModuleItem;

class CModuleDebugger : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CModuleDebugger, "Module Debugger", false)

public:
	CModuleDebugger() = default;

protected:
	virtual bool Shutdown() override;
	virtual void SetupUi() override;

	void UpdatePropertyList(CTreeModuleItem* item);
	void GenerateTree();

private:
	QTreeWidget* treeView;
	QWidget* Content;
	QScrollArea* PropertiesScroll;
	QWidget* Properties;

};
