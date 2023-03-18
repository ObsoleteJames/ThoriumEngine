
#include <string>
#include "Engine.h"
#include "ToolsWindow.h"
#include "EditorEngine.h"
#include "Registry/FileSystem.h"
#include <Util/KeyValue.h>
#include <Util/Assert.h>

#include <fstream>
#include <QMenuBar>
#include <QMouseEvent>
#include <QLabel>
#include <QBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>

//TArray<FToolsWindowClass*> _RegisteredWindows;
//TArray<FToolsWidgetClass*> _RegisteredWidgets;

TMap<SizeType, CToolsWindow*> _Windows;
TMap<SizeType, CToolsWidget*> _Widgets;

FString _StyleSheet;
	//"QMainWindow"
	//"{"
	//"background: #222;"
	//"border: 1px solid #222;"
	//"}"
	//"QMainWindow:active"
	//"{"
	//"	background: #1a191a;"
	//"	border: 1px solid #373737;"
	//"}"
	//"QMainWindow::separator"
	//"{"
	//"	width: 6px;"
	//"	background: transparent;"
	//"}"
	//"QMainWindow::separator:hover, QMainWindow::separator : pressed"
	//"{"
	//"	background: #2a5f7d;"
	//"}"
	//"QSplitter"
	//"{"
	//"	margin: 0px;"
	//"	background: transparent;"
	//"}"
	//"QSplitter::handle:horizontal"
	//"{"
	//"	width: 4px;"
	//"	background - color: #1a191a;"
	//"}"
	//"QSplitter::handle:vertical"
	//"{"
	//"	height:6px;"
	//"	background - color: #1a191a;"
	//"}"
	//"QSplitter::handle:hover, QSplitter::handle : pressed"
	//"{"
	//"	background: #2a5f7d;"
	//"}"
	//"QToolTip"
	//"{"
	//"	background: #333;"
	//"	color: #e0e0e0;"
	//"}";

CToolsWindow::CToolsWindow() : QMainWindow(nullptr)
{
	_Windows[ID] = this;

	_menuBar = new QMenuBar(this);
	_menuBar->setGeometry(0, 0, 0, 26);
	_menuBar->setObjectName("menuBar");
	_menuBar->setLayoutDirection(Qt::LeftToRight);
	setMenuBar(_menuBar);

	//setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);

}

CToolsWindow::~CToolsWindow()
{
	auto it = _Windows.find(ID);
	if (it != _Windows.end())
		_Windows.erase(it);

	//for (auto& it : BoundWidgets)
	//	delete it;

	//delete _titleBarWidget;
	//delete _windowTitle;
	//delete minimizeButton;
	//delete restoreButton;
	//delete maximizeButton;
	//delete closeButton;
}

FToolsWindowClass* CToolsWindow::GetClassById(SizeType _id)
{
	for (auto& wnd : ToolsRegisteredWindows::Get())
		if (wnd->Id == _id)
			return wnd;

	return nullptr;
}

CToolsWindow* CToolsWindow::CreateById(SizeType _id)
{
	for (const auto& wnd : ToolsRegisteredWindows::Get())
	{
		if (wnd->Id == _id)
			return wnd->Create();
	}
	return nullptr;
}

CToolsWindow* CToolsWindow::GetByName(const char* name)
{
	for (auto it : _Windows)
	{
		if (strcmp(it.second->Name, name) == 0)
			return it.second;
	}
	return nullptr;
}

bool CToolsWindow::CloseAll(CToolsWindow* ignore)
{
	if (_Windows.size() == 0)
		return true;

	for (auto it = _Windows.rbegin(); it != _Windows.rend(); it++)
	{
		if (it->second == ignore)
			continue;

		if (!it->second->Shutdown())
			return false;
		else
			it->second->destroy();
	}

	return true;
}

void CToolsWindow::ReloadStyle()
{
	_StyleSheet.Clear();
	for (auto& wnd : _Windows)
		wnd.second->setStyleSheet(GetStyleSheet().c_str());
}

FString& CToolsWindow::GetStyleSheet()
{
	if (_StyleSheet.IsEmpty())
	{
		FString stylePath = "editor\\themes\\" + gEditorEngine()->config.theme + "\\theme";

		FFile* file = CFileSystem::FindFile(ToWString(stylePath));
		THORIUM_ASSERT(file, "Failed to find theme" + gEditorEngine()->config.theme);

		FString enginePath = ToFString(file->Mod()->Path()) + "\\editor\\themes\\" + gEditorEngine()->config.theme;

		FKeyValue theme(file->FullPath());
		THORIUM_ASSERT(theme.IsOpen(), "Failed to open theme file '" + ToFString(file->FullPath()) + "'");

		for (auto& v : *theme.GetArray("include"))
		{
			std::ifstream includeStream((enginePath + "\\" + v).c_str());
			THORIUM_ASSERT(includeStream.is_open(), FString("Failed to open '") + v + "'");
			std::string _l;
			while (std::getline(includeStream, _l))
				_StyleSheet += _l + '\n';
		}

		//std::ifstream stream(file->FullPath().c_str());
		//THORIUM_ASSERT(stream.is_open(), "Failed to open theme file '" + ToFString(file->FullPath()) + "'");

		//std::string line;
		//while (std::getline(stream, line))
		//{
		//	size_t inclI = line.find('#');
		//	if (inclI != -1)
		//	{
		//		FString inclFile;
		//		const char* ptr = line.c_str() + inclI + 8;
		//		bool bInQuotes = false;
		//		while (ptr[0] != '\0')
		//		{
		//			char ch = ptr[0];
		//			if (ch == '\n')
		//				break;

		//			if (ch == '"')
		//			{
		//				bInQuotes ^= 1;
		//				ptr++;
		//				continue;
		//			}

		//			if (ch == ' ' && !bInQuotes)
		//			{
		//				ptr++;
		//				continue;
		//			}

		//			inclFile += ch;
		//			ptr++;
		//		}

		//		std::ifstream includeStream((enginePath + "\\bin\\styles\\" + inclFile).c_str());
		//		THORIUM_ASSERT(includeStream.is_open(), FString("Failed to open '") + inclFile + "'");
		//		std::string _l;
		//		while (std::getline(includeStream, _l))
		//		{
		//			_StyleSheet += _l + '\n';
		//		}
		//	}
		//}
	}

	return _StyleSheet;
}

bool CToolsWindow::CloseWindow()
{
	if (!Shutdown())
		return false;

	deleteLater();
	return true;
}

//void CToolsWindow::SetWindowTitle(const QString& title)
//{
//	setWindowTitle(title);
//	_windowTitle->setText(title);
//}

void CToolsWindow::SetupUi()
{
	setStyleSheet(GetStyleSheet().c_str());
	//QHBoxLayout* horizontalLayout = new QHBoxLayout();
	//horizontalLayout->setSpacing(0);
	//horizontalLayout->setMargin(0);

	//_titleBarWidget = new QWidget(this);
	//_titleBarWidget->setObjectName("titlebar");
	//_titleBarWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	//_titleBarWidget->setLayout(horizontalLayout);

	//minimizeButton = new QPushButton(_titleBarWidget);
	//minimizeButton->setObjectName("minimizeButton");
	//connect(minimizeButton, SIGNAL(clicked()), this, SLOT(slot_minimized()));

	//restoreButton = new QPushButton(_titleBarWidget);
	//restoreButton->setObjectName("restoreButton");
	//restoreButton->setVisible(false);
	//connect(restoreButton, SIGNAL(clicked()), this, SLOT(slot_restored()));

	//maximizeButton = new QPushButton(_titleBarWidget);
	//maximizeButton->setObjectName("maximizeButton");
	//connect(maximizeButton, SIGNAL(clicked()), this, SLOT(slot_maximized()));

	//closeButton = new QPushButton(_titleBarWidget);
	//closeButton->setObjectName("closeButton");
	//connect(closeButton, SIGNAL(clicked()), this, SLOT(slot_closed()));

	//_windowTitle = new QLabel(_titleBarWidget);
	//_windowTitle->setObjectName("windowTitle");
	//_windowTitle->setText(windowTitle());

	//menuBar = new QMenuBar(_titleBarWidget);
	//menuBar->setGeometry(0, 0, 0, 26);
	//menuBar->setObjectName("menuBar");
	//menuBar->setLayoutDirection(Qt::LeftToRight);

	//horizontalLayout->addWidget(menuBar);
	//horizontalLayout->addWidget(_windowTitle);
	//horizontalLayout->addStretch(1);
	//horizontalLayout->addWidget(minimizeButton);
	//horizontalLayout->addWidget(restoreButton);
	//horizontalLayout->addWidget(maximizeButton);
	//horizontalLayout->addWidget(closeButton);

	//QVBoxLayout* verticalLayout = new QVBoxLayout(this);
	//verticalLayout->setSpacing(0);
	//verticalLayout->setMargin(1);

	//verticalLayout->addWidget(_titleBarWidget);

	//QWidget* _centralWidget = new QWidget(this);
	//_centralWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//_centralWidget->setLayout(verticalLayout);

	//setCentralWidget(_centralWidget);

	/*for (auto& w : ToolsRegisteredWidgets::Get())
	{
		if (w->WindowHash == IdHash)
		{
			auto* widget = w->Create(this);
			widget->setHidden(!w->bShowOnStart);
			if (w->ToolBarPath)
			{
				// Split the ToolBarPath with '/' or '\\'
				auto split = FString(w->ToolBarPath).Split("/\\");

				QMenu* curMenu = nullptr;
				for (auto p = split.begin(); p != split.end(); p++)
				{
					if (QMenu* m = curMenu->findChild<QMenu*>(p->c_str())) {
						curMenu = m;
					}
					else if (p != split.last())
					{
						QMenu* prevMenu = curMenu;
						curMenu = new QMenu(this);
						curMenu->setObjectName(p->c_str());
						curMenu->setTitle(p->c_str());
						prevMenu->addMenu(curMenu);
					}
					else
					{
						QAction* action = new QAction(p->c_str());
						action->setObjectName(p->c_str());
						action->setCheckable(true);
						if (w->icon)
						{
							action->setIcon(*w->icon);
							action->setIconText(p->c_str());
						}

						curMenu->addAction(action);
						
						connect(widget, &QDockWidget::visibilityChanged, action, [=](bool b) { action->setChecked(b); });
						connect(action, &QAction::triggered, widget, [=](bool b) { if (b) widget->show(); else widget->hide(); });
					}
				}
			}

			BoundWidgets.Add(widget);
			widget->BindToWindow(this);
		}
	}*/

	//for (auto w : ToolsRegisteredWindows::Get())
	//{
	//	if (w->Id == IdHash)
	//		continue;

	//	if (w->ToolBarPath)
	//	{
	//		// Split the ToolBarPath with '/' or '\\'
	//		auto split = FString(w->ToolBarPath).Split("/\\");

	//		QMenu* curMenu = nullptr;
	//		if (QMenu* m = _menuBar->findChild<QMenu*>(split[0].c_str())) {
	//			curMenu = m;
	//		}
	//		else
	//		{
	//			curMenu = new QMenu(this);
	//			curMenu->setObjectName(split[0].c_str());
	//			curMenu->setTitle(split[0].c_str());
	//			_menuBar->addMenu(curMenu);
	//		}

	//		for (auto p = split.begin()++; p != split.end(); p++)
	//		{
	//			if (QMenu* m = curMenu->findChild<QMenu*>(p->c_str())) {
	//				curMenu = m;
	//			}
	//			else if (p != split.last())
	//			{
	//				QMenu* prevMenu = curMenu;
	//				curMenu = new QMenu(this);
	//				curMenu->setObjectName(p->c_str());
	//				curMenu->setTitle(p->c_str());
	//				prevMenu->addMenu(curMenu);
	//			}
	//			else
	//			{
	//				QAction* action = new QAction(p->c_str(), this);
	//				action->setObjectName(p->c_str());
	//				if (w->icon)
	//				{
	//					action->setIcon(*w->icon);
	//					action->setIconText(p->c_str());
	//				}

	//				curMenu->addAction(action);

	//				connect(action, &QAction::triggered, this, [=](bool) { w->Create(); });
	//			}
	//		}
	//	}
	//}

	setTabPosition(Qt::BottomDockWidgetArea, QTabWidget::North);
	setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::North);
	setTabPosition(Qt::RightDockWidgetArea, QTabWidget::North);
}

void CToolsWindow::SaveState()
{
	QString appdataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "\\..\\ThoriumEngine";
	QSettings settings(appdataPath + "\\EditorConfig\\" + Name + ".cfg", QSettings::Format::IniFormat);

	settings.setValue("window_state", saveState());
	settings.setValue("window_geo", saveGeometry());

	QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();

	for (auto* dock : dockWidgets)
	{
		settings.beginGroup(QString("dock_") + dock->objectName());

		settings.setValue("dockgeo", dock->saveGeometry());

		settings.endGroup();
	}

	UserSaveState(settings);
}

void CToolsWindow::RestoreState()
{
	QString appdataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "\\..\\ThoriumEngine";
	QSettings settings(appdataPath + "\\EditorConfig\\" + Name + ".cfg", QSettings::Format::IniFormat);

	restoreState(settings.value("window_state").toByteArray());
	restoreGeometry(settings.value("window_geo").toByteArray());

	QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();

	for (auto* dock : dockWidgets)
	{
		settings.beginGroup(QString("dock_") + dock->objectName());

		dock->restoreGeometry(settings.value("dockgeo").toByteArray());

		settings.endGroup();
	}

	UserRestoreState(settings);
}

//void CToolsWindow::mouseMoveEvent(QMouseEvent* event)
//{
//	if (!_titleBarWidget->underMouse() && !_windowTitle->underMouse())
//		return;
//
//	if (bMoving)
//	{
//		this->move(this->pos() + (event->pos() - lastMousePos));
//	}
//}
//
//void CToolsWindow::mousePressEvent(QMouseEvent* event)
//{
//	if (!_titleBarWidget->underMouse() && !_windowTitle->underMouse())
//		return;
//
//	if (event->button() == Qt::LeftButton)
//	{
//		bMoving = true;
//		lastMousePos = event->pos();
//	}
//}
//
//void CToolsWindow::mouseReleaseEvent(QMouseEvent *event)
//{
//	if (!_titleBarWidget->underMouse() && !_windowTitle->underMouse())
//		return;
//
//	if (event->button() == Qt::LeftButton)
//		bMoving = false;
//}
//
//void CToolsWindow::mouseDoubleClickEvent(QMouseEvent *event)
//{
//	Q_UNUSED(event);
//	if (!_titleBarWidget->underMouse() && !_windowTitle->underMouse())
//		return;
//
//	bMaximized = !bMaximized;
//	if (bMaximized)
//		slot_maximized();
//	else
//		slot_restored();
//}
//
//void CToolsWindow::slot_minimized()
//{
//	setWindowState(Qt::WindowMinimized);
//}
//
//void CToolsWindow::slot_restored()
//{
//	restoreButton->setVisible(false);
//	maximizeButton->setVisible(true);
//	setWindowState(Qt::WindowNoState);
//}
//
//void CToolsWindow::slot_maximized()
//{
//	restoreButton->setVisible(true);
//	maximizeButton->setVisible(false);
//	setWindowState(Qt::WindowMaximized);
//}
//
//void CToolsWindow::slot_closed()
//{
//	if (Shutdown())
//	{
//
//		close();
//	}
//}

TMap<SizeType, CToolsWindow*>& CToolsWindow::GetAll()
{
	return _Windows;
}

//CToolsWidget::CToolsWidget(CToolsWindow* parent) : QDockWidget(parent)
//{
//}
//
//CToolsWidget::~CToolsWidget()
//{
//}
//
//FToolsWidgetClass* CToolsWidget::GetClassById(SizeType _id)
//{
//	for (auto& wnd : ToolsRegisteredWidgets::Get())
//		if (wnd->Id == _id)
//			return wnd;
//
//	return nullptr;
//}

//void CToolsWidget::Register()
//{
//	//_Widgets[IdHash] = this;
//}

//TMap<SizeType, CToolsWidget*>& CToolsWidget::GetAll()
//{
//	return _Widgets;
//}
