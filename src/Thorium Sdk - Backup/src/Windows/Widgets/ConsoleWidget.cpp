
#include "ConsoleWidget.h"

#include <QLabel>
#include <QBoxLayout>
#include <QTextEdit>

const char* logTypeText[] = {
	"",
	"[INF]",
	"[WRN]",
	"[ERR]"
};

CConsoleWidget::CConsoleWidget(QWidget* parent /*= nullptr*/) : QDockWidget(parent)
{
	QWidget* rootWidget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout();
	rootWidget->setLayout(layout);

	setObjectName("console_widget");
	setWindowTitle("Console");

	setWidget(rootWidget);

	consoleLog = new QTextEdit(this);
	consoleLog->setReadOnly(true);
	layout->addWidget(consoleLog);

	consoleLog->setFontFamily("Consolas");

	FConsoleMsg* logPtr = CConsole::GetLinkedList();
	while (logPtr)
	{
		OnLog(*logPtr);
		logPtr = logPtr->next;
	}

	onLogBinding = CConsole::GetLogEvent().Bind(this, &CConsoleWidget::OnLog);
}

CConsoleWidget::~CConsoleWidget()
{
	CConsole::GetLogEvent().Remove(onLogBinding);
	delete consoleLog;
}

void CConsoleWidget::OnLog(const FConsoleMsg& msg)
{
	consoleLog->setTextColor(QColor(111, 179, 75));
	consoleLog->setTextBackgroundColor(QColor(111, 179, 75, 20));
	consoleLog->insertPlainText(logTypeText[msg.type]);
	
	consoleLog->setTextBackgroundColor(QColor(0, 0, 0, 0));
	consoleLog->setTextColor(QColor(200, 200, 200));

	consoleLog->insertPlainText(" ");

	if (msg.type == CONSOLE_WARNING)
		consoleLog->setTextColor(QColor(230, 197, 67));

	if (msg.type == CONSOLE_ERROR)
		consoleLog->setTextBackgroundColor(QColor(207, 32, 23, 100));

	consoleLog->insertPlainText((msg.msg + "\n").c_str());

}
