
#include <string>
#include "StringProperty.h"
#include "Object/Class.h"

#include <QVariant>
#include <QLineEdit>
#include <QBoxLayout>
#include <QLabel>

CStringProperty::CStringProperty(void* ptr, const FProperty* property, QWidget* parent /*= nullptr*/) : IBasePropertyEditor(parent)
{
	setProperty("type", QVariant(2));
	setLayout(new QHBoxLayout());

	if (property->typeName == "FString")
	{
		type = 0;
		fstring = (FString*)ptr;
	}
	else
	{
		type = 1;
		wstring = (WString*)ptr;
	}

	editor = new QLineEdit(this);

	QLabel* label = new QLabel(property->name.c_str(), this);

	layout()->addWidget(label);
	layout()->addWidget(editor);

	Update();
	connect(editor, &QLineEdit::textEdited, this, &CStringProperty::onEdit);
}

CStringProperty::CStringProperty(const FString& name, FString* ptr, QWidget* parent /*= nullptr*/) : IBasePropertyEditor(parent)
{
	setProperty("type", QVariant(2));
	setLayout(new QHBoxLayout());

	editor = new QLineEdit(this);

	type = 0;
	fstring = ptr;

	QLabel* label = new QLabel(name.c_str(), this);

	layout()->addWidget(label);
	layout()->addWidget(editor);

	Update();
	connect(editor, &QLineEdit::textEdited, this, &CStringProperty::onEdit);
}

CStringProperty::CStringProperty(const FString& name, WString* ptr, QWidget* parent /*= nullptr*/) : IBasePropertyEditor(parent)
{
	setProperty("type", QVariant(2));
	setLayout(new QHBoxLayout());

	editor = new QLineEdit(this);

	type = 1;
	wstring = ptr;

	QLabel* label = new QLabel(name.c_str(), this);

	layout()->addWidget(label);
	layout()->addWidget(editor);

	Update();
	connect(editor, &QLineEdit::textEdited, this, &CStringProperty::onEdit);
}

void CStringProperty::Update()
{
	switch (type)
	{
	case 0:
		if (editor->text() != fstring->c_str())
			editor->setText(fstring->c_str());
		break;
	case 1:
		if (editor->text() != wstring->c_str())
			editor->setText(QString((const QChar*)wstring->c_str()));
		break;
	}
}

void CStringProperty::onEdit(const QString& txt)
{
	switch (type)
	{
	case 0:
		*fstring = txt.toStdString();
		break;
	case 1:
		*wstring = txt.toStdWString();
		break;
	}
	emit(OnValueChanged());
}
