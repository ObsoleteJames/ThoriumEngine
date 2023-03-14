#pragma once

#include "Windows/ModelCreator/ModelCreator.h"
#include "Widgets/CollapsableWidget.h"
#include "Widgets/PropertyEditors/VectorProperty.h"
#include "Widgets/PropertyEditors/FloatProperty.h"

#include <QWidget>
#include <QFileDialog>
#include <QBoxLayout>
#include <QLineEdit>
#include <QPushButton>

const char* supportedFormats = "*.fbx;*.dea;*.obj;*.blend;*.md;*.smd;*.md2;*.md3";

class CMeshWidget : public CCollapsableWidget
{
public:
	CMeshWidget(CModelCreator* parent, FImportedMesh* d, SizeType id) : CCollapsableWidget(parent), mc(parent)
	{
		SetHeaderType(CCollapsableWidget::NESTED_HEADER);
		this->id = id;

		GetHeader()->setMinimumHeight(28);

		QHBoxLayout* headerLayout = new QHBoxLayout(GetHeader());
		headerLayout->setContentsMargins(32, 2, 2, 2);
		
		nameEdit = new QLineEdit(this);
		nameEdit->setPlaceholderText("Name...");
		connect(nameEdit, &QLineEdit::editingFinished, this, [=]() { data->name = nameEdit->text().toStdString().c_str(); mc->MarkDirty(); });
		
		QPushButton* removeBtn = new QPushButton("X", this);
		removeBtn->setProperty("type", QVariant("clear"));
		connect(removeBtn, &QPushButton::clicked, this, [=]() { mc->RemoveMesh(this->id); });

		headerLayout->addWidget(nameEdit);
		headerLayout->addWidget(removeBtn);

		QWidget* w = new QWidget(this);
		QVBoxLayout* l = new QVBoxLayout(w);
		l->setContentsMargins(4, 0, 0, 0);
		l->setSpacing(0);
		SetWidget(w);

		// Mesh File
		{
			QFrame* frame = new QFrame(w);
			QHBoxLayout* l2 = new QHBoxLayout(frame);
			frame->setProperty("type", QVariant(2));

			pathEdit = new QLineEdit(frame);
			pathEdit->setReadOnly(true);
			QPushButton* browseBtn = new QPushButton("Browse", frame);

			connect(browseBtn, &QPushButton::clicked, this, [=]() {
				QString file = QFileDialog::getOpenFileName(this, "Open Mesh File", QString(), QString("Mesh File (") + supportedFormats + ")");
				if (file.isEmpty())
					return;

				pathEdit->setText(file);
				data->file = file.toStdWString().c_str();
				mc->UpdateMesh(this->id);
				UpdateUI();
			});

			l2->addWidget(pathEdit);
			l2->addWidget(browseBtn);

			l->addWidget(frame);
		}

		// Transform
		{
			propPosition = new CVectorProperty(&d->position, "Position", w);
			propRotation = new CVectorProperty(&d->rotation, "Rotation", w);
			propScale = new CFloatProperty("Scale", &d->scale, w);

			l->addWidget(propPosition);
			l->addWidget(propRotation);
			l->addWidget(propScale);
		}

		SetData(d);
	}

	void UpdateUI()
	{
		WString headerName = data->file;
		if (!headerName.IsEmpty())
		{
			if (auto it = headerName.FindLastOf(L"\\/"); it != -1)
				headerName.Erase(headerName.begin(), headerName.begin() + it + 1);

			SetText(QString((const QChar*)headerName.c_str()));
		}
		else
			SetText("Empty Mesh");

		pathEdit->setText(QString((const QChar*)data->file.c_str()));
		nameEdit->setText(data->name.c_str());

		propPosition->Update();
		propRotation->Update();
		propScale->Update();
	}

	void SetData(FImportedMesh* newData)
	{
		data = newData;
		propPosition->SetValue(&data->position);
		propRotation->SetValue(&data->rotation);
		propScale->SetValue(&data->scale);
		UpdateUI();
	}

public:
	SizeType id;

private:
	CVectorProperty* propPosition;
	CVectorProperty* propRotation;
	CFloatProperty* propScale;

	QLineEdit* nameEdit;
	QLineEdit* pathEdit;
	FImportedMesh* data;
	CModelCreator* mc;
};
