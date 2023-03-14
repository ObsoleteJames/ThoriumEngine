#pragma once

#include "ToolsWindow.h"
#include <Util/String.h>

class QListWidget;

struct FProjectDef
{
	FString name;
	WString path;

	bool bHasIcon = false;
};

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

	void SearchForProjects();
	void UpdateProjectList();

	void CreateNewProject();
	void OpenProject();

private:
	TArray<FProjectDef> projects;
	QListWidget* projectList;

};
