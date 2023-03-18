
#include "SaveDialog.h"
#include <QBoxLayout>
#include <QPushButton>
#include <QVariant>
#include <QLabel>

CSaveDialog::CSaveDialog(QWidget* parent /*= nullptr*/, Qt::WindowFlags f /*= Qt::WindowFlags()*/) : CFramelessDialog(parent)
{
	QFrame* frame = new QFrame(this);
	QVBoxLayout* layout = new QVBoxLayout(frame);
	//{
	//	QVBoxLayout* l = new QVBoxLayout(this);
	//	l->setContentsMargins(0, 0, 0, 0);
	//	l->addWidget(frame);
	//}
	setCentralWidget(frame);

	QHBoxLayout* lButtons = new QHBoxLayout();
	
	label = new QLabel("Continue without saving?", this);

	QPushButton* btnSave = new QPushButton("Save", this);
	QPushButton* btnDontSave = new QPushButton("Don't Save", this);
	QPushButton* btnCancel = new QPushButton("Cancel", this);

	btnSave->setProperty("type", QVariant("primary"));

	lButtons->addWidget(btnSave);
	lButtons->addWidget(btnDontSave);
	lButtons->addWidget(btnCancel);

	layout->addWidget(label);
	layout->addLayout(lButtons);

	resize(250, 120);

	connect(btnSave, &QPushButton::clicked, this, [=]() { done(SAVE_SUCCESS); deleteLater(); });
	connect(btnDontSave, &QPushButton::clicked, this, [=]() { done(SAVE_IGNORE); deleteLater(); });
	connect(btnCancel, &QPushButton::clicked, this, [=]() { done(SAVE_CANCEL); deleteLater(); });
}

CSaveDialog::~CSaveDialog()
{
}

void CSaveDialog::SetText(const QString& txt)
{
	label->setText(txt);
}
