
#include "MaterialEditor.h"
#include "Game/World.h"
#include "Game/Entity.h"
#include "Game/Components/ModelComponent.h"
#include "Game/Components/CameraComponent.h"
#include "Rendering/RenderScene.h"
#include "Widgets/WorldViewportWidget.h"
#include "Widgets/SaveDialog.h"
#include "Widgets/CollapsableWidget.h"
#include "Widgets/FileDialogs.h"
#include "Widgets/ObjectSelectorWidget.h"

#include "Widgets/PropertyEditors/ArrayProperty.h"
#include "Widgets/PropertyEditors/IntProperty.h"
#include "Widgets/PropertyEditors/FloatProperty.h"
#include "Widgets/PropertyEditors/BoolProperty.h"
#include "Widgets/PropertyEditors/EnumProperty.h"
#include "Widgets/PropertyEditors/StringProperty.h"
#include "Widgets/PropertyEditors/StructProperty.h"
#include "Widgets/PropertyEditors/ObjectPtrProperty.h"
#include "Widgets/PropertyEditors/VectorProperty.h"

#include <QMenuBar>
#include <QPaintEvent>
#include <QBoxLayout>
#include <QDockWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QScrollArea>
#include <QColorDialog>

SDK_REGISTER_WINDOW(CMaterialEditor, "Material Editor", "Tools", NULL);

class CMatTextureProperty : public IBasePropertyEditor
{
public:
	CMatTextureProperty(const FString& name, CMaterial::MatTexture* t, QWidget* parent) : IBasePropertyEditor(parent), tex(t)
	{
		setProperty("type", QVariant(2));
		auto* layout = new QVBoxLayout(this);

		QLabel* label = new QLabel(name.c_str(), this);
		layout->addWidget(label);
		
		assetWidget = new QWidget(this);

		auto* l2 = new QHBoxLayout(assetWidget);
		l2->setMargin(0);
		
		edit = new CObjectSelectorWidget(CTexture::StaticClass(), this);
		edit->setMinimumWidth(180);
		
		QPushButton* browse = new QPushButton("Browse", this);
		browse->setMaximumWidth(80);

		l2->addWidget(edit);
		l2->addWidget(browse);

		layout->addWidget(assetWidget);
		
		colorWidget = new QWidget(this);

		auto* l3 = new QHBoxLayout(colorWidget);
		l2->setMargin(0);

		QLineEdit* colorEdit = new QLineEdit(colorWidget);
		QPushButton* btnPick = new QPushButton(colorWidget);
		btnPick->setMaximumWidth(80);

		l3->addWidget(colorEdit);
		l3->addWidget(btnPick);

		layout->addWidget(colorWidget);

		QPushButton* btnSwitch = new QPushButton(this);
		layout->addWidget(btnSwitch);

		assetWidget->setVisible(!t->bIsCustom);
		colorWidget->setVisible(t->bIsCustom);

		connect(browse, &QPushButton::clicked, this, [=]() {
			COpenFileDialog dialog((FAssetClass*)CTexture::StaticClass(), this);
			if (dialog.exec() && dialog.File())
			{
				TObjectPtr<CTexture> obj = CResourceManager::GetResource((FAssetClass*)CTexture::StaticClass(), dialog.File()->Path());
				t->tex = obj;
				edit->SetObject(obj);
				emit(OnValueChanged());
			}
		});
		connect(edit, &CObjectSelectorWidget::ObjectChanged, this, [=]() {
			CTexture* obj = CastChecked<CTexture>(edit->GetObject());
			t->tex = obj;
			emit(OnValueChanged());
		});
		connect(btnPick, &QPushButton::clicked, this, [=]() {
			QColor col;
			col.setRed(t->color[0]);
			col.setGreen(t->color[1]);
			col.setBlue(t->color[2]);
			col.setAlpha(t->color[3]);
			
			QColor newCol = QColorDialog::getColor(col, this);
			t->color[0] = newCol.red();
			t->color[1] = newCol.green();
			t->color[2] = newCol.blue();
			t->color[3] = newCol.alpha();

			uint8 data[] = {
					t->color[0], t->color[1], t->color[2], t->color[3],
					t->color[0], t->color[1], t->color[2], t->color[3],
					t->color[0], t->color[1], t->color[2], t->color[3],
					t->color[0], t->color[1], t->color[2], t->color[3]
			};

			if (!t->tex)
				t->tex = CreateObject<CTexture>();
			t->tex->Init(data, 2, 2);
		});

		connect(btnSwitch, &QPushButton::clicked, this, [=]() {
			if (!t->bIsCustom && t->tex)
			{
				texPath = t->tex->File()->Path();
			}

			t->bIsCustom = !t->bIsCustom;
			if (t->bIsCustom)
			{
				uint8 data[] = {
					t->color[0], t->color[1], t->color[2], t->color[3],
					t->color[0], t->color[1], t->color[2], t->color[3],
					t->color[0], t->color[1], t->color[2], t->color[3],
					t->color[0], t->color[1], t->color[2], t->color[3]
				};

				t->tex = CreateObject<CTexture>();
				t->tex->Init(data, 2, 2);
			}
			else if (!texPath.IsEmpty())
			{
				t->tex = CResourceManager::GetResource((FAssetClass*)CTexture::StaticClass(), texPath);
			}
			assetWidget->setVisible(!t->bIsCustom);
			colorWidget->setVisible(t->bIsCustom);
			emit(OnValueChanged());
		});

		Update();
	}

	void Update()
	{
		if (!tex)
			return;

		if (!tex->bIsCustom)
		{
			CTexture* t = tex->tex;
			if (t != edit->GetObject())
				edit->SetObject(t);
		}
	}
	
private:
	WString texPath;

	CObjectSelectorWidget* edit;
	QWidget* assetWidget;
	QWidget* colorWidget;
	CMaterial::MatTexture* tex;

};

bool CMaterialEditor::Shutdown()
{
	if (bRequiresSave && !ShutdownSave())
		return false;

	SaveState();
	return true;
}

void CMaterialEditor::SetupUi()
{
	CToolsWindow::SetupUi();

	{
		QMenu* menuFile = new QMenu("File", this);
		QMenu* menuEdit = new QMenu("Edit", this);

		menuFile->addAction("New", this, &CMaterialEditor::NewMaterial, QKeySequence(QKeySequence::New));
		menuFile->addAction("Open", this, &CMaterialEditor::LoadMaterial, QKeySequence(QKeySequence::Open));
		menuFile->addAction("Save", this, [=]() { this->SaveMaterial(); }, QKeySequence(QKeySequence::Save));

		_menuBar->addMenu(menuFile);
		_menuBar->addMenu(menuEdit);
	}

	setWindowTitle("Material Editor");

	QWidget* widget = new QWidget(this);
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	widget->setLayout(layout);

	setCentralWidget(widget);

	viewport = new CWorldViewportWidget(widget);
	layout->addWidget(viewport);

	{
		editorWidget = new QDockWidget(this);
		editorWidget->setWindowTitle("Material");
		editorWidget->setObjectName("mat_editor");

		QWidget* widget = new QWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(widget);
		layout->setSpacing(0);
		editorWidget->setWidget(widget);

		shaderEditor = new CObjectPtrProperty("Shader", nullptr, CShaderSource::StaticClass(), widget);
		shaderEditor->AllowNull(false);
		shaderEditor->setMaximumHeight(42);
		layout->addWidget(shaderEditor);

		shaderEditor->GetSelector()->SetAssetValidator([](const WString& path) { 
			//CShaderSource* shader = Cast<CShaderSource>(obj);
			CShaderSource* shader = CResourceManager::GetResource<CShaderSource>(path);
			if (!shader)
				return false;

			if (shader->type != CShaderSource::ST_FORWARD && shader->type != CShaderSource::ST_DEFERRED && shader->type != CShaderSource::ST_DEBUG)
				return false;

			return true;
		});

		connect(shaderEditor, &CObjectPtrProperty::OnValueChanged, this, [=]() { material->Validate(); UpdateUI(); });

		QScrollArea* scrollArea = new QScrollArea(this);
		scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		scrollArea->setWidgetResizable(true);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		layout->addWidget(scrollArea);

		QFrame* scrollWidget = new QFrame(this);
		contentLayout = new QVBoxLayout(scrollWidget);
		scrollWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
		contentLayout->setMargin(0);
		contentLayout->setSpacing(0);
		scrollArea->setWidget(scrollWidget);

		addDockWidget(Qt::LeftDockWidgetArea, editorWidget);
	}

	RestoreState();

	Init();
	UpdateUI();
}

void CMaterialEditor::Init()
{
	world = CreateObject<CWorld>();
	world->InitWorld(CWorld::InitializeInfo().CreateAISystems(false).CreatePhyiscsWorld(false).RegisterForRendering(false));
	viewport->SetOverrideScene(world->GetRenderScene());

	TObjectPtr<CEntity> modelEnt = world->CreateEntity<CEntity>();
	modelComp = modelEnt->AddComponent<CModelComponent>("Model");
	modelComp->SetModel(L"models\\Sphere.thmdl");

	camera = CreateObject<CCameraComponent>();
	viewport->SetControlMode(ECameraControlMode::Orbit);
	viewport->camera = camera;
	world->GetRenderScene()->SetCamera(camera);
}

void CMaterialEditor::paintEvent(QPaintEvent* event)
{
	world->Update(viewport->GetDeltaTime());
	world->Render();

	viewport->Render();

	if (isActiveWindow())
		update(viewport->rect());

	event->accept();
}

void CMaterialEditor::closeEvent(QCloseEvent* event)
{
	if (!CloseWindow())
	{
		event->ignore();
		return;
	}

	material = nullptr;
	camera->Delete();
	world->Delete();
	world = nullptr;

	event->accept();
}

bool CMaterialEditor::ShutdownSave()
{
	CSaveDialog saveDialog(this);
	int r = saveDialog.exec();

	if (r == CSaveDialog::SAVE_SUCCESS)
	{
		SaveMaterial();
		return true;
	}

	return r;
}

void CMaterialEditor::SetMaterial(CMaterial* mat, bool bNew)
{
	if (bRequiresSave && material)
	{
		if (!ShutdownSave())
			return;
	}

	bRequiresSave = bNew;

	bReadOnly = false;
	material = mat;
	modelComp->SetMaterial(material);
	UpdateUI();
	UpdateTitle();
}

void CMaterialEditor::SaveMaterial(bool bNew)
{
	if (!material)
		return;

	if (!material->File())
	{
		CSaveFileDialog dialog(this);
		if (!dialog.exec())
			return;

		CResourceManager::RegisterNewResource(material, dialog.Path());
	}

	material->Save();
	bRequiresSave = false;
	UpdateTitle();
}

void CMaterialEditor::NewMaterial()
{
	TObjectPtr<CMaterial> newMat = CreateObject<CMaterial>();
	newMat->SetShader("Simple");
	
	SetMaterial(newMat, true);
}

void CMaterialEditor::LoadMaterial()
{
	if (bRequiresSave && material)
	{
		if (!ShutdownSave())
			return;
	}

	COpenFileDialog dialog((FAssetClass*)CMaterial::StaticClass(), this);
	if (!dialog.exec())
		return;

	TObjectPtr<CMaterial> newMat = CResourceManager::GetResource<CMaterial>(dialog.File()->Path());
	SetMaterial(newMat);
}

void CMaterialEditor::UpdateUI()
{
	for (auto* w : curWidgets)
		contentLayout->removeWidget(w);
	curWidgets.Clear();

	if (!material)
	{
		editorWidget->widget()->setDisabled(true);
		setWindowTitle("Material Editor");
		return;
	}

	shaderEditor->SetValue(&material->shader);

	for (auto& prop : material->textures)
	{
		FShaderTexture* shaderTex = material->GetShaderTexture(prop);
		CCollapsableWidget* catWidget = GetHeader(shaderTex->UiGroup);

		CMatTextureProperty* edit = new CMatTextureProperty(shaderTex->displayName, &prop, catWidget);

		catWidget->Widget()->layout()->addWidget(edit);
		connect(edit, &CMatTextureProperty::OnValueChanged, this, [=]() { bRequiresSave = true; UpdateTitle(); });

	}

	for (auto& prop : material->properties)
	{
		FShaderProperty* shaderProp = material->GetShaderProperty(prop);
		CCollapsableWidget* catWidget = GetHeader(shaderProp->UiGroup);

		IBasePropertyEditor* edit = nullptr;

		switch (shaderProp->type)
		{
		case FShaderProperty::BOOL:
			edit = new CBoolProperty(shaderProp->displayName, (bool*)&prop.pBool, catWidget);
			break;
		case FShaderProperty::INT:
			edit = new CIntProperty(shaderProp->displayName, &prop.pInt, 0, 0, catWidget);
			break;
		case FShaderProperty::FLOAT:
			edit = new CFloatProperty(shaderProp->displayName, &prop.pFloat, catWidget);
			break;
		case FShaderProperty::VEC3:
			edit = new CVectorProperty((FVector*)prop.pVec3, shaderProp->displayName, catWidget);
			break;
		case FShaderProperty::VEC4:
			break;
		}

		if (edit)
		{
			catWidget->Widget()->layout()->addWidget(edit);
			CMaterial::MatProperty* p = &prop;
			connect(edit, &IBasePropertyEditor::OnValueChanged, this, [=]() { p->bRequiresUpdate = true; bRequiresSave = true; UpdateTitle(); });
		}
	}

	editorWidget->widget()->setDisabled(bReadOnly);
	UpdateTitle();
}

void CMaterialEditor::UpdateTitle()
{
	if (!material)
	{
		setWindowTitle("Material Editor");
		return;
	}

	if (material->File())
	{
		FFile* file = material->File();

		WString title = L"Material Editor - " + file->Name();
		if (bRequiresSave)
			title += L'*';
		if (bReadOnly)
			title += L" (Read Only)";

		setWindowTitle(QString((const QChar*)title.c_str()));
	}
	else
	{
		WString title = L"Material Editor - New Material";
		if (bRequiresSave)
			title += L'*';

		setWindowTitle(QString((const QChar*)title.c_str()));
	}
}

CCollapsableWidget* CMaterialEditor::GetHeader(const FString& category)
{
	for (auto* c : curWidgets)
	{
		if (c->Text() == category.c_str())
			return c;
	}
	
	CCollapsableWidget* r = new CCollapsableWidget(category.c_str(), nullptr, this);
	QWidget* content = new QWidget(r);
	QVBoxLayout* layout = new QVBoxLayout(content);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	r->SetWidget(content);
	r->SetCollapsed(false);

	curWidgets.Add(r);
	contentLayout->addWidget(r);
	return r;
}
