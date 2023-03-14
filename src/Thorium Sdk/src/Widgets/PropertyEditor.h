#pragma once

#include <QFrame>
#include <Util/Map.h>
#include "Object/Object.h"
#include "Widgets/TreeDataItem.h"

class QScrollArea;
class QVBoxLayout;
class CCollapsableWidget;

class IBasePropertyEditor : public QFrame
{
	Q_OBJECT

public:
	IBasePropertyEditor(QWidget* parent) : QFrame(parent) {}

	virtual QWidget* GetWidget() { return this; }
	virtual void Update() = 0;

Q_SIGNALS:
	void OnValueChanged();

};

class CPropertyEditorWidget : public QWidget
{
	Q_OBJECT

public:
	CPropertyEditorWidget(QWidget* parent = nullptr);

	void SetObject(CObject* obj);
	inline TObjectPtr<CObject> GetObject() const { return targetObject; }

	inline void SetReadOnly(bool b) { bReadOnly = b; RebuildUI(); }
	inline bool ReadOnly() const { return bReadOnly; }

	static IBasePropertyEditor* CreatePropertyEditor(void* ptr, const FProperty* p, QWidget* parent);

	// Updates all property values;
	void Update();

private:
	void RebuildUI();

	CCollapsableWidget* GetCategoryWidget(const FString& category);

	void AddProperties(FClass* type, CObject* obj, const FString& overrideCat = FString());

protected:
	QScrollArea* scrollArea;
	QVBoxLayout* scrollLayout;
	TArray<CCollapsableWidget*> curWidgets;
	TArray<IBasePropertyEditor*> properties;
	TObjectPtr<CObject> targetObject;
	bool bReadOnly;

};
