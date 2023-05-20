
#include "DataAssetEditor.h"
#include "Resources/DataAsset.h"
#include "Widgets/PropertyEditor.h"
#include "Widgets/SaveDialog.h"
#include "Widgets/FileDialogs.h"
#include "Widgets/ObjectSelectorWidget.h"
#include "Widgets/ClassSelectorDialog.h"
#include "Widgets/CollapsableWidget.h"
#include "Console.h"

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
#include <QMenu>
#include <QBoxLayout>
#include <QScrollArea>
#include <QCloseEvent>

SDK_REGISTER_WINDOW(CDataAssetEditor, "Data Asset Editor", "Tools", NULL);

bool CDataAssetEditor::Shutdown()
{
	if (bRequiresSave && !ShutdownSave())
		return false;

	SaveState();
	return true;
}

void CDataAssetEditor::SetupUi()
{
	CToolsWindow::SetupUi();

	{
		QMenu* menuFile = new QMenu("File", this);
		
		menuFile->addAction("New", this, &CDataAssetEditor::New, QKeySequence(QKeySequence::New));
		menuFile->addAction("Open", this, &CDataAssetEditor::Open, QKeySequence(QKeySequence::Open));
		menuFile->addAction("Save", this, &CDataAssetEditor::Save, QKeySequence(QKeySequence::Save));

		_menuBar->addMenu(menuFile);
	}

	QWidget* widget = new QWidget(this);
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout* layout = new QVBoxLayout(widget);
	layout->setSpacing(0);

	setCentralWidget(widget);

	QScrollArea* scrollArea = new QScrollArea(this);
	scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	layout->addWidget(scrollArea);

	QFrame* scrollWidget = new QFrame(this);
	l2 = new QVBoxLayout(scrollWidget);
	scrollWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
	l2->setMargin(0);
	l2->setSpacing(0);
	scrollArea->setWidget(scrollWidget);

	QRect r = geometry();
	r.setWidth(418);
	r.setHeight(520);
	setGeometry(r);

	RestoreState();

	UpdateUI();
}

void CDataAssetEditor::closeEvent(QCloseEvent* event)
{
	if (!CloseWindow())
	{
		event->ignore();
		return;
	}

	asset = nullptr;

	event->accept();
}

bool CDataAssetEditor::ShutdownSave()
{
	CSaveDialog saveDialog(this);
	int r = saveDialog.exec();

	if (r == CSaveDialog::SAVE_SUCCESS)
	{
		Save();
		return true;
	}

	return r;
}

void CDataAssetEditor::SetAsset(CDataAsset* asset, bool bNew /*= false*/)
{
	if (bRequiresSave && asset)
	{
		if (!ShutdownSave())
			return;
	}

	bRequiresSave = bNew;

	bReadOnly = false;
	this->asset = asset;
	UpdateUI();
}

void CDataAssetEditor::Save(bool bNew /*= false*/)
{
	if (!asset)
		return;

	if (!asset->File())
	{
		CSaveFileDialog dialog(this);
		if (!dialog.exec())
			return;

		if (!CResourceManager::RegisterNewResource(asset, dialog.Path()))
			CONSOLE_LogError("CDataAssetEditor", "Failed to register new resource '" + ToFString(dialog.Path()) + "'");
	}

	asset->Save();
	bRequiresSave = false;
	UpdateTitle();
}

void CDataAssetEditor::New()
{
	if (bRequiresSave && asset)
	{
		if (!ShutdownSave())
			return;

		bRequiresSave = false;
	}

	CClassSelectorDialog dialog(this);
	dialog.SetFilterClass(CDataAsset::StaticClass());

	if (!dialog.exec())
		return;

	asset = (CDataAsset*)dialog.GetSelectedClass()->Instantiate();
	bRequiresSave = asset != nullptr;
	bReadOnly = false;
	UpdateUI();
}

void CDataAssetEditor::Open()
{
	if (bRequiresSave && asset)
	{
		if (!ShutdownSave())
			return;
	}

	COpenFileDialog dialog((FAssetClass*)CDataAsset::StaticClass(), this);
	if (!dialog.exec())
		return;

	asset = (TObjectPtr<CDataAsset>)CResourceManager::GetResource(nullptr, dialog.File()->Path());
	bRequiresSave = false;
	bReadOnly = false;
	UpdateUI();
}

void CDataAssetEditor::UpdateUI()
{
	for (auto* w : curWidgets)
		l2->removeWidget(w);
	curWidgets.Clear();

	if (!asset)
	{
		UpdateTitle();
		return;
	}

	for (FClass* t = asset->GetClass(); t != nullptr && t != CDataAsset::StaticClass(); t = t->GetBaseClass())
	{
		for (const FProperty* p = t->GetPropertyList(); p != nullptr; p = p->next)
		{
			if ((p->flags & VTAG_EDITOR_EDITABLE) == 0 && (p->flags & VTAG_EDITOR_VISIBLE) == 0)
				continue;

			bool readOnly = p->flags & VTAG_EDITOR_VISIBLE;

			FString catName = t->GetName();
			if (p->meta && !p->meta->category.IsEmpty())
				catName = p->meta->category;

			CCollapsableWidget* cat = GetHeader(catName);

			void* ptr = (void*)((SizeType)&*asset + p->offset);

			IBasePropertyEditor* editor = CPropertyEditorWidget::CreatePropertyEditor(ptr, p, cat);

			if (editor)
			{
				if (readOnly)
					editor->setEnabled(false);

				QVBoxLayout* l = (QVBoxLayout*)cat->Widget()->layout();
				l->insertWidget(0, editor);

				connect(editor, &IBasePropertyEditor::OnValueChanged, this, [=]() {
					if (p->meta)
					{
						const FFunction* f = t->GetFunction(p->meta->FlagValue("OnEditFunc"));
						if (f)
						{
							FStack s(0);
							f->execFunc(asset, s);
							//p->meta->onEditFunc(obj, s);
						}
					}
					bRequiresSave = true;
					UpdateTitle();
				});
			}
		}
	}

	UpdateTitle();
}

void CDataAssetEditor::UpdateTitle()
{
	if (!asset)
	{
		setWindowTitle("Data Asset Editor");
		return;
	}

	if (asset->File())
	{
		FFile* file = asset->File();

		WString title = L"Data Asset Editor - " + file->Name();
		if (bRequiresSave)
			title += L'*';
		if (bReadOnly)
			title += L" (Read Only)";

		setWindowTitle(QString((const QChar*)title.c_str()));
	}
	else
	{
		WString title = L"Data Asset Editor - New File";
		if (bRequiresSave)
			title += L'*';

		setWindowTitle(QString((const QChar*)title.c_str()));
	}
}

CCollapsableWidget* CDataAssetEditor::GetHeader(const FString& category)
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
	l2->addWidget(r);
	return r;
}
