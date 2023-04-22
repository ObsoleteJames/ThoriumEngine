
#include "ConsoleWidget.h"

#include <QLabel>
#include <QBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QSplitter>
#include <QPushButton>

const char* logTypeText[] = {
	"",
	"[INF]",
	"[WRN]",
	"[ERR]"
};

CConsoleWidget::CConsoleWidget(QWidget* parent /*= nullptr*/) : ads::CDockWidget("Console", parent)
{
	QWidget* rootWidget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout();
	rootWidget->setLayout(layout);

	setObjectName("console_widget");
	setWidget(rootWidget);

	splitter = new QSplitter(Qt::Horizontal, this);
	layout->addWidget(splitter);

	QWidget* consoleText = new QWidget(this);
	QVBoxLayout* l2 = new QVBoxLayout(consoleText);

	consoleLog = new QTextEdit(this);
	consoleLog->setReadOnly(true);
	l2->addWidget(consoleLog);

	consoleLog->setFontFamily("Consolas");

	input = new QLineEdit(this);
	input->setProperty("type", QVariant(1));
	input->setFont(consoleLog->font());

	l2->addWidget(input);

	splitter->addWidget(consoleText);

	QWidget* msgInfo = new QWidget(this);
	//msgInfo->setStyleSheet("QPushButton { background-color: rgba(62, 98, 196, 64); border-radius: 0; border: 1 solid #6786D3; }");
	QVBoxLayout* l3 = new QVBoxLayout(msgInfo);
	l3->setMargin(0);
	l3->setSpacing(0);

	msgFunction = new QPushButton(msgInfo);
	msgFunction->setProperty("type", QVariant("clear"));
	msgFile = new QPushButton(msgInfo);
	msgFile->setProperty("type", QVariant("clear"));
	msgLine = new QPushButton(msgInfo);
	msgLine->setProperty("type", QVariant("clear"));

	l3->addWidget(msgFunction);
	l3->addWidget(msgFile);
	l3->addWidget(msgLine);
	l3->addStretch(1);
	splitter->addWidget(msgInfo);
	splitter->setStretchFactor(0, 7);
	splitter->setStretchFactor(1, 1);

#if CONSOLE_USE_ARRAY
	const TArray<FConsoleMsg>& msgCache = CConsole::GetMsgCache();
	for (auto& msg : msgCache)
		OnLog(msg);
#else
	FConsoleMsg* logPtr = CConsole::GetLinkedList();
	while (logPtr)
	{
		OnLog(*logPtr);
		logPtr = logPtr->next;
	}
#endif

	onLogBinding = CConsole::GetLogEvent().Bind(this, &CConsoleWidget::OnLog);

	connect(input, &QLineEdit::returnPressed, this, [=]() { CConsole::Exec(input->text().toStdString()); input->clear(); });
	connect(consoleLog, &QTextEdit::cursorPositionChanged, this, [=]() {
		SetSelectedMsg(consoleLog->textCursor().blockNumber());
	});
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

	if (msg.type != CONSOLE_PLAIN)
		consoleLog->insertPlainText(("[" + msg.module + "]").c_str());

	consoleLog->setTextColor(QColor(200, 200, 200));
	consoleLog->setTextBackgroundColor(QColor(0, 0, 0, 0));
	consoleLog->insertPlainText(" ");

	if (msg.type == CONSOLE_PLAIN)
		consoleLog->setTextColor(QColor(150, 150, 150));
	else if (msg.type == CONSOLE_WARNING)
		consoleLog->setTextColor(QColor(230, 197, 67));
	else if (msg.type == CONSOLE_ERROR)
		consoleLog->setTextBackgroundColor(QColor(207, 32, 23, 100));

	consoleLog->insertPlainText((msg.msg + "\n").c_str());
	consoleLog->moveCursor(QTextCursor::End);
}

void CConsoleWidget::SetSelectedMsg(int index)
{
	const TArray<FConsoleMsg>& msgCache = CConsole::GetMsgCache();
	if (index == -1 || msgCache.Size() <= index)
		return;

	const FConsoleMsg& msg = msgCache[index];
	msgFunction->setText(QString("Function: ") + msg.info.function);
	msgFile->setText(QString("File: ") + msg.info.file);
	msgLine->setText(QString("Line: ") + QString::number(msg.info.line));
}
