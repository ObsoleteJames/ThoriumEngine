
#include <string>
#include "EditorWindow.h"
#include "EditorEngine.h"
#include "Window.h"
#include "Game/Events.h"
#include "Game/World.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Misc/TransformGizmoEntity.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderCommands.h"
#include "Widgets/WorldViewportWidget.h"
#include "Widgets/RenderWidget.h"
#include "Widgets/ClassSelectorDialog.h"
#include "Widgets/PropertyEditor.h"
#include "Widgets/FileDialogs.h"
#include "Widgets/SaveDialog.h"
#include "Windows/Debug/ModuleDebugger.h"
#include "Windows/Debug/WidgetDemo.h"
#include "Windows/Debug/ObjectDebugger.h"
#include "Windows/ModelCreator/ModelCreator.h"
#include "Windows/MaterialEditor.h"
#include "Resources/Scene.h"
#include "Resources/Material.h"
#include "AssetBrowserDock.h"
#include "ConsoleWidget.h"
#include "OutlinerWidget.h"
#include "PropertiesWidget.h"
#include "EditorMode.h"
#include <Util/KeyValue.h>

#include <fstream>
#include <QCloseEvent>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>

SDK_REGISTER_WINDOW(CEditorWindow, "Editor Window", NULL, NULL);

void RegisterAssetMenus()
{
	CAssetBrowserWidget::RegisterAssetCreateMenu("Create Shader", [](const WString& path) {
		CToolsWindow* wnd = CToolsWindow::GetByName("Editor Window");
		CFramelessDialog* dialog = new CFramelessDialog(wnd);

		QFrame* frame = new QFrame(dialog);
		QVBoxLayout* layout = new QVBoxLayout(frame);

		dialog->setCentralWidget(frame);
		dialog->setTitle("Create Shader");

		QLineEdit* nameEdit = new QLineEdit(wnd);
		nameEdit->setPlaceholderText("Name...");

		layout->addWidget(nameEdit);

		QHBoxLayout* l1 = new QHBoxLayout();

		QPushButton* btnImport = new QPushButton("Create", frame);
		btnImport->setProperty("type", QVariant("primary"));
		btnImport->setEnabled(false);
		QPushButton* btnCancel = new QPushButton("Cancel", frame);

		l1->addWidget(btnImport);
		l1->addWidget(btnCancel);

		layout->addLayout(l1);

		QObject::connect(nameEdit, &QLineEdit::textChanged, wnd, [=](const QString& str) { btnImport->setEnabled(!str.isEmpty()); });
		QObject::connect(nameEdit, &QLineEdit::returnPressed, wnd, [=]() { dialog->done(true); });

		QObject::connect(btnImport, &QPushButton::clicked, wnd, [=]() { dialog->done(true); });
		QObject::connect(btnCancel, &QPushButton::clicked, wnd, [=]() { dialog->done(false); });

		if (dialog->exec() && !nameEdit->text().isEmpty())
		{
			WString mod = path;
			WString dir = path;

			if (auto it = mod.FindFirstOf(':'); mod.begin() + it != mod.end())
			{
				mod.Erase(mod.begin() + it, mod.end());

				if (it + 2 < dir.Size())
					dir.Erase(dir.begin(), dir.begin() + it + 2);
				else
					dir.Clear();
			}

			FString shaderName = nameEdit->text().toStdString();
			TObjectPtr<CShaderSource> shader = CResourceManager::CreateResource<CShaderSource>(dir + L"\\" + ToWString(shaderName) + ToWString(((FAssetClass*)CShaderSource::StaticClass())->GetExtension()), mod);

			std::ofstream sdkStream(shader->File()->GetSdkPath().c_str());
			if (!sdkStream.is_open())
				return;

			sdkStream << ("\nShader\n{\n\tName = \"" + shaderName + "\";\n\tType = SHADER_FORWARD;\n}\n\n").c_str()
				<< "Global\n{\n\t#include \"common/common.hlsl\"\n\n\tstruct VS_Input\n\t{\n\t\t#include \"common/vertex_input.hlsl\"\n\t};\n\n\tstruct PS_Input\n\t{\n\t\t#include \"common/pixel_input.hlsl\"\n\t};\n\n\tProperty<int> vTestProperty(Name = \"Test Property\");\n}\n\n"
				<< "VS\n{\n\t#include \"common/vertex.hlsl\"\n\n\tPS_Input Main(VS_Input input)\n\t{\n\t\tPS_Input output = ProcessVertex(input);\n\n\t\tFinalizeVertex(input, output);\n\t\treturn output;\n\t}\n}\n\n"
				<< "PS\n{\n\tfloat4 Main(PS_Input input) : SV_TARGET\n\t{\n\t\treturn float4(0.5f, 0.5f, 1.f, 1.f);\n\t}\n}\n";

			sdkStream.close();

			shader->Compile();
			shader->Init();
		}
	});
}

CEditorWindow::CEditorWindow()
{
	int x = QApplication::desktop()->screenGeometry().width();
	int y = QApplication::desktop()->screenGeometry().height();

	x -= 1600;
	y -= 900;
	x /= 2;
	y /= 2;
	setGeometry(QRect(x, y, 1600, 900));

	gEditorEngine()->editorWindow = this;
	gEngine->LoadWorld(L"empty", true);

	connect(&gEditorEngine()->historyBuffer, &CHistoryBuffer::onEventAdded, this, [=]() {
		gEditorEngine()->bSceneDirty = true;
		UpdateTitle();
	});
	connect(&gEditorEngine()->historyBuffer, &CHistoryBuffer::onUndo, this, [=](SizeType cursor) {
		if (cursor == gEditorEngine()->savedAtHC)
			gEditorEngine()->bSceneDirty = false;
		else
			gEditorEngine()->bSceneDirty = true;
		UpdateTitle();
	});
	connect(&gEditorEngine()->historyBuffer, &CHistoryBuffer::onRedo, this, [=](SizeType cursor) {
		if (cursor == gEditorEngine()->savedAtHC)
			gEditorEngine()->bSceneDirty = false;
		else
			gEditorEngine()->bSceneDirty = true;
		UpdateTitle();
	});
}

CEditorWindow::~CEditorWindow()
{
	delete worldViewport;
	delete consoleWidget;
}

bool CEditorWindow::Shutdown()
{
	gEditorEngine()->OnObjectSelected.Remove(this);
	Events::PostLevelChange.Remove(this);

	SaveState();
	return true;
}

void CEditorWindow::SetupUi()
{
	CToolsWindow::SetupUi();

	// Setup Menubar
	{
		menuFile = new QMenu("File", _menuBar);
		menuEdit = new QMenu("Edit", _menuBar);
		menuView = new QMenu("View", _menuBar);
		menuTools = new QMenu("Tools", _menuBar);
		menuDebug = new QMenu("Debug", _menuBar);
		menuFile->setObjectName("File");
		menuEdit->setObjectName("Edit");
		menuView->setObjectName("View");
		menuTools->setObjectName("Tools");
		menuDebug->setObjectName("Debug");

		_menuBar->addMenu(menuFile);
		_menuBar->addMenu(menuEdit);
		_menuBar->addMenu(menuTools);
		_menuBar->addMenu(menuView);
		_menuBar->addMenu(menuDebug);

		menuFile->addAction("New Scene", this, &CEditorWindow::NewScene, QKeySequence(QKeySequence::New));
		menuFile->addAction("Open Scene", this, &CEditorWindow::LoadScene, QKeySequence(QKeySequence::Open));

		menuFile->addAction("Save", this, [=]() { SaveScene(false); }, QKeySequence(QKeySequence::Save));
		menuFile->addAction("Save As", this, [=]() { SaveScene(true); }, QKeySequence(QKeySequence::SaveAs));

		menuFile->addSeparator();
		menuFile->addAction("Quit", this, []() { QApplication::quit(); });

		menuEdit->addAction("Undo", this, [=]() { gEditorEngine()->historyBuffer.Undo(); }, QKeySequence(QKeySequence::Undo));
		menuEdit->addAction("Redo", this, [=]() { gEditorEngine()->historyBuffer.Redo(); }, QKeySequence(QKeySequence::Redo));

		menuEdit->addSeparator();

		menuEdit->addAction("Create Entity", this, [=]() {
			CClassSelectorDialog* dialog = new CClassSelectorDialog(this);
			connect(dialog, &QDialog::finished, this, [=](int result) { 
				if (!result)
					return;
				FClass* type = dialog->GetSelectedClass();
				CEntity* ent = gWorld->CreateEntity(dialog->GetSelectedClass(), FString());

				class FEntCreateEvent : public IHistoryEvent
				{
				public:
					FEntCreateEvent(CEntity* _ent) : ent(_ent)
					{
						world = ent->GetWorld();
						type = ent->GetClass();
						name = "Created Entity: " + _ent->Name();
					}

					void Undo()
					{
						ent->Delete();
						ent = nullptr;
					}

					void Redo()
					{
						ent = world->CreateEntity(type, FString());
					}

					CWorld* world;
					FClass* type;
					TObjectPtr<CEntity> ent;
				};

				gEditorEngine()->historyBuffer.AddEvent(new FEntCreateEvent(ent));
			});

			dialog->SetFilterClass(CEntity::StaticClass());
			dialog->exec();
		});
		
		//menuDebug->addAction("Module Debugger", this, []() { CModuleDebugger::Class()->Create(); });
		//menuDebug->addAction("Object Debugger", this, []() { CObjectDebugger::Class()->Create(); });
		//menuDebug->addAction("Widget Demo", this, []() { CWidgetDemo::Class()->Create(); });
		
		//menuTools->addAction("Model Creator", this, []() { CModelCreator::Class()->Create(); });
		//menuTools->addAction("Material Editor", this, []() { CMaterialEditor::Class()->Create(); });
		menuDebug->addAction("Reload StyleSheet", this, []() { CToolsWindow::ReloadStyle(); });
		menuDebug->addSeparator();
	}

	SetupMenuBar();
	UpdateTitle();

	setWindowIcon(QIcon(":/icons/thorium engine icon.svg"));

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

	// Toolbar
	{
		toolbar = new QToolBar(this);
		toolbar->setObjectName("mainToolBar");
		toolbar->setMinimumHeight(42);

		QPushButton* saveBtn = new QPushButton(gEditorEngine()->GetIcon(L"floppy.svg"), QString(), toolbar);
		saveBtn->setProperty("type", QVariant("clear"));
		saveBtn->setMinimumSize(QSize(20, 20));
		saveBtn->setMaximumSize(QSize(24, 24));
		connect(saveBtn, &QPushButton::clicked, this, [=]() { SaveScene(); });
		toolbar->addWidget(saveBtn);
		toolbar->addSeparator();

		editorModeCB = new QComboBox(toolbar);
		editorModeCB->setMinimumWidth(130);

		for (auto* m : CEditorModeRegistry::Get())
			editorModeCB->addItem(m->Icon(), QString(m->Name().c_str()));

		connect(editorModeCB, (void(QComboBox::*)(const QString&)) & QComboBox::currentIndexChanged, this, [=](const QString& t) { gEditorEngine()->SetEditorMode(t.toStdString()); });

		toolbar->addWidget(editorModeCB);
		toolbar->addSeparator();

		QFrame* worldBtnsFrame = new QFrame(toolbar);
		QHBoxLayout* l2 = new QHBoxLayout(worldBtnsFrame);
		worldBtnsFrame->setMaximumHeight(26);
		worldBtnsFrame->setStyleSheet("QFrame { border-radius: 3px; }");
		l2->setSpacing(4);
		l2->setMargin(4);

		QPushButton* btnPlay = new QPushButton(gEditorEngine()->GetIcon(L"btn-play.svg"), QString(), toolbar);
		QPushButton* btnStop = new QPushButton(gEditorEngine()->GetIcon(L"btn-stop.svg"), QString(), toolbar);

		btnPlay->setMaximumWidth(20);
		btnPlay->setProperty("type", QVariant("clear"));
		btnStop->setMaximumWidth(20);
		btnStop->setProperty("type", QVariant("clear"));

		l2->addWidget(btnPlay);
		l2->addWidget(btnStop);

		connect(btnPlay, &QPushButton::clicked, this, [=]() { 
			if (gEditorEngine()->bIsPlaying)
				return;

			if ((gEditorEngine()->bSceneDirty || !gWorld->GetScene()->File()) && !SaveScene())
				return;
			gEditorEngine()->bIsPlaying = true;
			gWorld->Start();
		});

		connect(btnStop, &QPushButton::clicked, this, [=]() {
			if (!gEditorEngine()->bIsPlaying)
				return;

			gEditorEngine()->bIsPlaying = false;
			gWorld->Stop();
		});

		toolbar->addWidget(worldBtnsFrame);

		addToolBar(Qt::TopToolBarArea, toolbar);
	}

	worldViewport = new CWorldViewportWidget(widget);
	layout->addWidget(worldViewport);

	worldViewport->SetCamera(gEditorEngine()->editorCamera);

	consoleWidget = new CConsoleWidget(this);
	addDockWidget(Qt::BottomDockWidgetArea, consoleWidget);

	assetBrowser = new CAssetBrowserDW(this);
	addDockWidget(Qt::BottomDockWidgetArea, assetBrowser);

	outlinerWidget = new COutlinerWidget(this);
	addDockWidget(Qt::RightDockWidgetArea, outlinerWidget);

	/*QDockWidget* propertiesDock = new QDockWidget(this);
	propertiesWidget = new CPropertyEditorWidget(this);
	propertiesDock->setWidget(propertiesWidget);
	propertiesDock->setWindowTitle("Properties");
	propertiesDock->setObjectName("properties_widget");
	addDockWidget(Qt::RightDockWidgetArea, propertiesDock);*/
	propertiesWidget = new CPropertiesWidget(this);
	addDockWidget(Qt::RightDockWidgetArea, propertiesWidget);

	menuView->addAction(consoleWidget->toggleViewAction());
	menuView->addAction(assetBrowser->toggleViewAction());
	menuView->addAction(outlinerWidget->toggleViewAction());
	menuView->addAction(propertiesWidget->toggleViewAction());

	QMenu* themesMenu = new QMenu("Themes", this);
	menuView->addSeparator();
	menuView->addMenu(themesMenu);

	for (auto& th : gEditorEngine()->GetThemes())
		themesMenu->addAction(th.displayName.c_str(), this, [=]() { gEditorEngine()->SetTheme(th.name); CToolsWindow::ReloadStyle(); }, QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T));

	//gEditorEngine()->SetRenderScene(worldViewport->GetRenderScene());
	//gWorld->SetRenderScene(worldViewport->GetRenderScene());
	worldViewport->SetOverrideScene(gWorld->GetRenderScene());
	gWorld->SetRenderWindow(worldViewport->GetWindow());

	//setWindowState(Qt::WindowMaximized);
	//centralWidget()->layout()->addWidget(widget);

	connect(outlinerWidget, &COutlinerWidget::entitySelected, this, [=](CEntity* ent) { gEditorEngine()->SetSelectedObject(ent); });
	connect(propertiesWidget, &CPropertiesWidget::ComponentSelected, this, [=](TObjectPtr<CObject> obj) { gEditorEngine()->SetSelectedObject(obj); });

	gEditorEngine()->OnObjectSelected.Bind(this, &CEditorWindow::OnObjectSelected);

	RestoreState();
	RegisterAssetMenus();

	Events::PostLevelChange.Bind(this, &CEditorWindow::OnLevelChange);
	
	worldViewport->GetWindow()->OnKeyEvent.Bind(this, [=](EKeyCode key, EInputAction action, EInputMod mod) {
		switch (key)
		{
		case EKeyCode::ESCAPE:
			gEditorEngine()->SetSelectedObject(nullptr);
			break;
		}
	});

	CONSOLE_LogInfo("CEditorEngine", "Created Editor Window");
}

void CEditorWindow::SetupMenuBar()
{
	for (auto w : ToolsRegisteredWindows::Get())
	{
		if (w->Id == GetId() || !w->ToolBarPath)
			continue;

		// Split the ToolBarPath with '/' or '\\'
		auto split = FString(w->ToolBarPath).Split("/\\");

		QMenu* curMenu = nullptr;
		if (QMenu* m = _menuBar->findChild<QMenu*>(split[0].c_str())) {
			curMenu = m;
		}
		else
		{
			curMenu = new QMenu(split[0].c_str(), _menuBar);
			curMenu->setObjectName(split[0].c_str());
			//curMenu->setTitle();
			_menuBar->addMenu(curMenu);
		}

		for (auto p = split.begin()++; p != split.end(); p++)
		{
			if (QMenu* m = curMenu->findChild<QMenu*>(p->c_str())) {
				curMenu = m;
				continue;
			}

			QMenu* prevMenu = curMenu;
			curMenu = new QMenu(prevMenu);
			curMenu->setObjectName(p->c_str());
			curMenu->setTitle(p->c_str());
			prevMenu->addMenu(curMenu);
		}

		QAction* action = new QAction(w->Name, this);
		action->setObjectName(w->Name);
		if (w->icon)
		{
			action->setIcon(*w->icon);
			action->setIconText(w->Name);
		}

		curMenu->addAction(action);

		connect(action, &QAction::triggered, this, [=](bool) { w->Create(); });
	}
}

bool CEditorWindow::AttempSave()
{
	CSaveDialog* saveDialog = new CSaveDialog(this);
	int r = saveDialog->exec();

	if (r == CSaveDialog::SAVE_SUCCESS)
	{
		SaveScene();
		return true;
	}

	return r;
}

void CEditorWindow::NewScene()
{
	if (gEditorEngine()->bSceneDirty && !AttempSave())
		return;
	
	outlinerWidget->Clear();
	propertiesWidget->SetObject(nullptr);
	gEngine->LoadWorld(L"empty", true);

	gEditorEngine()->bSceneDirty = true;
	gEditorEngine()->savedAtHC = -1;
	gEditorEngine()->historyBuffer.ClearHistory();
	worldViewport->SetCamera(gEditorEngine()->editorCamera);
	gWorld->SetRenderWindow(worldViewport->GetWindow());
	
	UpdateTitle();
}

void CEditorWindow::LoadScene()
{
	if (gEditorEngine()->bSceneDirty && !AttempSave())
		return;

	COpenFileDialog* dialog = new COpenFileDialog((FAssetClass*)CScene::StaticClass(), this);
	if (!dialog->exec())
		return;

	outlinerWidget->Clear();
	propertiesWidget->SetObject(nullptr);
	gEngine->LoadWorld(dialog->File()->Path(), true);

	CFStream sdkStream = dialog->File()->GetSdkStream("rb");
	if (sdkStream.IsOpen())
	{
		CCameraComponent* cam = gEditorEngine()->editorCamera;
		FVector camPos;
		FQuaternion camRot;

		sdkStream >> &camPos >> &camRot;

		cam->SetWorldPosition(camPos);
		cam->SetWorldRotation(camRot);
		sdkStream.Close();
	}

	worldViewport->SetCamera(gEditorEngine()->editorCamera);
	gWorld->SetRenderWindow(worldViewport->GetWindow());
	gEditorEngine()->bSceneDirty = false;
	gEditorEngine()->savedAtHC = -1;
	gEditorEngine()->historyBuffer.ClearHistory();

	UpdateTitle();
}

bool CEditorWindow::SaveScene(bool bNewPath /*= false*/)
{
	if (!gWorld->GetScene()->File())
	{
		CSaveFileDialog dialog(this);
		if (!dialog.exec())
			return false;

		CResourceManager::RegisterNewResource(gWorld->GetScene(), dialog.Path());
		gWorld->Save();
		return true;
	}
	if (bNewPath)
	{
		// TODO
		return false;
	}

	CFStream sdkStream = gWorld->GetScene()->File()->GetSdkStream("wb");
	if (sdkStream.IsOpen())
	{
		CCameraComponent* cam = gEditorEngine()->editorCamera;
		FVector camPos = cam->GetWorldPosition();
		FQuaternion camRot = cam->GetWorldRotation();

		sdkStream << &camPos << &camRot;
		sdkStream.Close();
	}

	gWorld->Save();
	gEditorEngine()->bSceneDirty = false;
	gEditorEngine()->savedAtHC = gEditorEngine()->historyBuffer.cursor;
	UpdateTitle();
	return true;
}

void CEditorWindow::closeEvent(QCloseEvent *event)
{
	if (gEditorEngine()->bSceneDirty && !AttempSave())
	{
		event->ignore();
		return;
	}

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
	static int inactiveCounter = 0;
	if (!isActiveWindow())
	{
		inactiveCounter++;

		if (inactiveCounter < 50)
		{
			update(worldViewport->rect());
			event->accept();
			return;
		}

		inactiveCounter = 0;
	}

	gEditorEngine()->SetDeltaTime(worldViewport->GetDeltaTime());
	gEngine->Run();

	outlinerWidget->Update();
	propertiesWidget->Update();

	// Do Gizmo raycast
	//{
	//	const FVector center(0.f, 0.5f, 0.f);
	//	const FVector cDir(0.f, 1.f, 0.f);

	//	double mx, my;
	//	FRay mouseRay{};
	//	worldViewport->GetWindow()->GetMousePos(mx, my);

	//	mouseRay = gEditorEngine()->editorCamera->MouseToRay(mx, my, worldViewport->GetWindow());

	//	//FVector& p = gEditorEngine()->gizmoPos;
	//	//p = mouseRay.origin + mouseRay.direction;

	//	bool bHit;
	//	double hitDist;
	//	FMath::RayCylinderIntersection(center, cDir, 0.035, 1.0, mouseRay.origin, mouseRay.direction, bHit, hitDist);

	//	gEditorEngine()->gizmo2Mat->SetFloat("Variable1", (float)bHit);
	//}

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

	update(worldViewport->rect());

	event->accept();
}

void CEditorWindow::UserSaveState(QSettings& out)
{
	out.beginGroup("asset_browser");
	out.setValue("viewmode", (int)assetBrowser->assetBrowser->ViewMode());
	out.setValue("gridsize", assetBrowser->assetBrowser->GridSize());
	out.setValue("splitter", assetBrowser->assetBrowser->GetSplitter()->saveState());
	out.endGroup();

	out.beginGroup("property_editor");
	out.setValue("splitter_geo", propertiesWidget->split->saveState());
	out.endGroup();

	out.beginGroup("console_widget");
	out.setValue("slpitter_state", consoleWidget->Splitter()->saveState());
	out.endGroup();
}

void CEditorWindow::UserRestoreState(QSettings& in)
{
	in.beginGroup("asset_browser");
	assetBrowser->assetBrowser->SetViewMode(CAssetBrowserWidget::EViewMode(in.value("viewmode").toInt()));
	assetBrowser->assetBrowser->SetGridSize(in.value("gridsize").toInt());
	assetBrowser->assetBrowser->GetSplitter()->restoreState(in.value("splitter").toByteArray());
	in.endGroup();

	in.beginGroup("property_editor");
	propertiesWidget->split->restoreState(in.value("splitter_geo").toByteArray());
	in.endGroup();

	in.beginGroup("console_widget");
	consoleWidget->Splitter()->restoreState(in.value("splitter_state").toByteArray());
	in.endGroup();
}

void CEditorWindow::UpdateTitle()
{
	WString title = L"Thorium Engine";

	if (gWorld->GetScene()->File())
		title += L" - " + gWorld->GetScene()->File()->Name();
	else
		title += L" - New Scene";

	if (gEditorEngine()->bSceneDirty)
		title += L'*';

	setWindowTitle(QString((const QChar*)title.c_str()));
}

void CEditorWindow::OnLevelChange()
{
	worldViewport->SetOverrideScene(gWorld->GetRenderScene());
}

void CEditorWindow::OnObjectSelected(const TArray<TObjectPtr<CObject>>& objs)
{
	if (objs.Size() == 0)
		propertiesWidget->SetObject(nullptr);
	else
		propertiesWidget->SetObject(objs[0]);
}
