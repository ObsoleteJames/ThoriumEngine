#pragma once

#include "EngineCore.h"
#include <functional>
#include "FramelessDialog.h"

class FClass;
class QTreeWidget;
class QPushButton;
class QLineEdit;

class CCreateCppClassDialog : public CFramelessDialog
{
	Q_OBJECT

public:
	CCreateCppClassDialog(QWidget* parent = nullptr);
	virtual ~CCreateCppClassDialog();

	inline void SetFilterClass(FClass* c) { filterClass = c; }
	inline FClass* GetSelectedClass() const { return selectedClass; }

	int exec() override;

private:
	void AddClassItem(FClass* c, void* parent);

	void Finish();

private:
	FClass* selectedClass;
	FClass* filterClass;

	QTreeWidget* classTree;
	QPushButton* selectButton;
	QPushButton* cancelButton;
	QLineEdit* nameEdit;
	QLineEdit* pathEdit;
};
