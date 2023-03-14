#pragma once

#include <Util/Core.h>
#include "Widgets/PropertyEditor.h"

struct FProperty;
class QLineEdit;

class CStringProperty : public IBasePropertyEditor
{
	Q_OBJECT

public:
	CStringProperty(void* ptr, const FProperty* property, QWidget* parent = nullptr);
	CStringProperty(const FString& name, FString* ptr, QWidget* parent = nullptr);
	CStringProperty(const FString& name, WString* ptr, QWidget* parent = nullptr);

	void Update();

	void SetValue(FString* ptr) { type = 0; fstring = ptr; Update(); }
	void SetValue(WString* ptr) { type = 1; wstring = ptr; Update(); }

private:
	void onEdit(const QString&);

private:
	QLineEdit* editor;
	uint8 type;
	union
	{
		FString* fstring;
		WString* wstring;
	};

};
