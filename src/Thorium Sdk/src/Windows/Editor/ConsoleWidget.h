#pragma once

#include <QDockWidget>
#include "Console.h"

class QLineEdit;
class QTextEdit;
class QSplitter;
class QPushButton;

class CConsoleWidget : public QDockWidget
{
	Q_OBJECT

public:
	CConsoleWidget(QWidget* parent = nullptr);
	virtual ~CConsoleWidget();

	inline QSplitter* Splitter() const { return splitter; }

private:
	void OnLog(const FConsoleMsg& msg);

	void SetSelectedMsg(int index);

private:
	QTextEdit* consoleLog;
	QLineEdit* input;

	QSplitter* splitter;

	QPushButton* msgFunction;
	QPushButton* msgFile;
	QPushButton* msgLine;

	SizeType onLogBinding;
};
