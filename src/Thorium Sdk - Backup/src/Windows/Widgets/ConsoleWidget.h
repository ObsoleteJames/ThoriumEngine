#pragma once

#include <QDockWidget>
#include "Console.h"

class QTextEdit;

class CConsoleWidget : public QDockWidget
{
	Q_OBJECT

public:
	CConsoleWidget(QWidget* parent = nullptr);
	virtual ~CConsoleWidget();

private:
	void OnLog(const FConsoleMsg& msg);

private:
	QTextEdit* consoleLog;
	SizeType onLogBinding;
};
