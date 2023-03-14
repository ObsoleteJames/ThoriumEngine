#pragma once

#include "FramelessDialog.h"
#include <Util/Core.h>

class QLabel;

class CSaveDialog : public CFramelessDialog
{
	Q_OBJECT

public:
	enum EResult
	{
		SAVE_SUCCESS = 1,
		SAVE_IGNORE = 2,
		SAVE_CANCEL = 0,
	};

public:
	CSaveDialog(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
	virtual ~CSaveDialog();

	void SetText(const QString& txt);
	inline void SetText(const FString& txt) { SetText(QString(txt.c_str())); }

private:
	QLabel* label;

};
