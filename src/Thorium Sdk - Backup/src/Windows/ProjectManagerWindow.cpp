
#include "ProjectManagerWindow.h"
#include <QPainter>

SDK_REGISTER_WINDOW(CProjectManagerWnd, "Project Manager", NULL, NULL);

CProjectManagerWnd::CProjectManagerWnd()
{
}

CProjectManagerWnd::~CProjectManagerWnd()
{

}

bool CProjectManagerWnd::Shutdown()
{
	return true;
}

void CProjectManagerWnd::SetupUi()
{
	CToolsWindow::SetupUi();
	
}
