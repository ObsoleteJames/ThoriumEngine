
#include "WidgetDemo.h"
#include <QPainter>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QSlider>
#include <QPushButton>

SDK_REGISTER_WINDOW(CWidgetDemo, "Widget Demo", "Debug", NULL);

void CWidgetDemo::SetupUi()
{
	CToolsWindow::SetupUi();

	setWindowTitle("Widget Demo");
	
	QWidget* widget = new QWidget(this);
	widget->setLayout(new QVBoxLayout());

	setCentralWidget(widget);

	QSlider* slider = new QSlider(Qt::Horizontal, this);
	slider->setMinimum(2);
	slider->setMaximum(1024 * 1024);

	QPushButton* btnAllocate = new QPushButton("Allocate", this);
	connect(btnAllocate, &QPushButton::clicked, this, [=]() { 
		arr.Resize(slider->value()); 
	});

	QPushButton* btnFree = new QPushButton("Free", this);
	connect(btnFree, &QPushButton::clicked, this, [=]() { 
		arr.~TArr();
		new (&arr) TArr();
		arr.Resize(2);
	});

	widget->layout()->addWidget(slider);
	widget->layout()->addWidget(btnAllocate);
	widget->layout()->addWidget(btnFree);
}
