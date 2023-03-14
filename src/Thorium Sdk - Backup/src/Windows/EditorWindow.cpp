
#include "EditorWindow.h"
#include "EditorEngine.h"
#include "Game/Events.h"
#include "Game/World.h"
#include "Game/Components/CameraComponent.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderCommands.h"
#include "Widgets/ConsoleWidget.h"
#include "Widgets/AssetBrowser/AssetBrowser.h"
#include "Widgets/WorldViewportWidget.h"
#include "Widgets/RenderWidget.h"
#include "Debug/ModuleDebugger.h"

#include <QCloseEvent>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QDesktopWidget>

SDK_REGISTER_WINDOW(CEditorWindow, "Editor Window", NULL, NULL);

CEditorWindow::CEditorWindow()
{
	int x = QApplication::desktop()->screenGeometry().width();
	int y = QApplication::desktop()->screenGeometry().height();

	x -= 1600;
	y -= 900;
	x /= 2;
	y /= 2;
	setGeometry(QRect(x, y, 1600, 900));

	gEngine = new CEditorEngine();
	gEngine->Init();

	// Create StyleReloadEvent.
	QAction* styleReloadEvent = new QAction("Reload StyleSheet", this);
	styleReloadEvent->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T));
	connect(styleReloadEvent, &QAction::triggered, this, [=]() { CToolsWindow::ReloadStyle(); statusBar()->showMessage("Reloaded style sheet", 4); });
	addAction(styleReloadEvent);

}

CEditorWindow::~CEditorWindow()
{
	delete worldViewport;
	delete consoleWidget;

	gEditorEngine()->OnExit();
	delete gEngine;
}

bool CEditorWindow::Shutdown()
{
	SaveState();
	return true;
}

void CEditorWindow::SetupUi()
{
	CToolsWindow::SetupUi();

	// Setup Menubar
	{
		menuFile = new QMenu("File", this);
		menuEdit = new QMenu("Edit", this);
		menuView = new QMenu("View", this);
		menuTools = new QMenu("Tools", this);
		menuDebug = new QMenu("Debug", this);

		_menuBar->addMenu(menuFile);
		_menuBar->addMenu(menuEdit);
		_menuBar->addMenu(menuTools);
		_menuBar->addMenu(menuView);
		_menuBar->addMenu(menuDebug);

		menuFile->addAction("New Scene", this, []() {}, QKeySequence("Ctrl+N"));
		menuFile->addAction("Open Scene", this, []() {}, QKeySequence("Ctrl+P"));

		menuFile->addAction("Save Current", this, []() {}, QKeySequence("Ctrl+S"));
		menuFile->addAction("Save All", this, []() {}, QKeySequence("Ctrl+Shift+S"));
		menuFile->addAction("Save As", this, []() {}, QKeySequence("Ctrl+Alt+S"));

		menuFile->addSeparator();
		menuFile->addAction("Quit", this, []() { QApplication::quit(); });
		
		menuDebug->addAction("Module Debugger", this, []() { CModuleDebugger::Class()->Create(); });

		menuTools->addAction("Model Creator", this, []() { });
		menuTools->addAction("Material Editor", this, []() {});
	}

	setWindowTitle("Thorium Editor");

	QWidget* widget = new QWidget(this);
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(1);

	widget->setLayout(layout);
	setCentralWidget(widget);

	QStatusBar* statusBar = new QStatusBar(this);
	setStatusBar(statusBar);

	FString projectName = FString("Project: ") + gEngine->GetProjectConfig().name;

	projectLabel = new QLabel(this);
	projectLabel->setText(projectName.c_str());

	fpsLabel = new QLabel(this);

	statusBar->addPermanentWidget(projectLabel, 1);
	statusBar->addPermanentWidget(fpsLabel, 5);

	// Cursor Mode
	{
		QAction* cmSelect = new QAction("Select", this);
		QAction* cmTranslate = new QAction("Translate", this);
		QAction* cmRotate = new QAction("Rotate", this);
		QAction* cmScale = new QAction("Scale", this);
		cmSelect->setCheckable(true);
		cmTranslate->setCheckable(true);
		cmRotate->setCheckable(true);
		cmScale->setCheckable(true);

		QActionGroup* cursorModeGroup = new QActionGroup(this);
		cursorModeGroup->addAction(cmSelect);
		cursorModeGroup->addAction(cmTranslate);
		cursorModeGroup->addAction(cmRotate);
		cursorModeGroup->addAction(cmScale);
		cmSelect->setChecked(true);

		QToolBar* cursorModeBar = new QToolBar(this);
		//cursorModeBar->addWidget(new QLabel("Cursor: "));
		cursorModeBar->addAction(cmSelect);
		cursorModeBar->addAction(cmTranslate);
		cursorModeBar->addAction(cmRotate);
		cursorModeBar->addAction(cmScale);
		addToolBar(Qt::TopToolBarArea, cursorModeBar);
	}

	// SelectionMode
	{
		
	}

	worldViewport = new CWorldViewportWidget(widget);
	layout->addWidget(worldViewport);

	worldViewport->camera = gEditorEngine()->editorCamera;
	worldViewport->GetRenderScene()->SetCamera(gEditorEngine()->editorCamera);

	consoleWidget = new CConsoleWidget(this);
	addDockWidget(Qt::BottomDockWidgetArea, consoleWidget);

	assetBrowser = new CAssetBrowserWidget(this);
	addDockWidget(Qt::BottomDockWidgetArea, assetBrowser);

	gEditorEngine()->SetRenderScene(worldViewport->GetRenderScene());
	gWorld->SetRenderScene(worldViewport->GetRenderScene());

	//setWindowState(Qt::WindowMaximized);
	//centralWidget()->layout()->addWidget(widget);

	RestoreState();

	CONSOLE_LogInfo("CEditorWindow::SetupUi done");
}

void CEditorWindow::closeEvent(QCloseEvent *event)
{
	if (CToolsWindow::CloseAll(this))
	{
		event->accept();
		Shutdown();
		QApplication::quit();
	}
	else
		event->ignore();
}

void CEditorWindow::paintEvent(QPaintEvent *event)
{
	gEditorEngine()->SetDeltaTime(worldViewport->GetDeltaTime());
	gEngine->Run();
	worldViewport->Render();
	
	static int counter = 0;
	if (counter == 15)
	{
		fpsAvarage /= 15.0;
		fpsLabel->setText(QString("FPS: ") + QString::number(1.0 / fpsAvarage));
		fpsAvarage = 0;
		counter = 0;
	}
	else
	{
		fpsAvarage += worldViewport->GetDeltaTime();
		counter++;
	}

	if (isActiveWindow())
		update();

	event->accept();
}
