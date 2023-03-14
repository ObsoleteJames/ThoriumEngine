#pragma once

#include <QDockWidget>
#include <Util/Map.h>
#include "Object/Object.h"
#include "Widgets/TreeDataItem.h"

class QLineEdit;
class QTreeWidget;
class CEntity;
class CWorld;

class COutlinerWidget : public QDockWidget
{
	Q_OBJECT

public:
	COutlinerWidget(QWidget* parent = nullptr);
	virtual ~COutlinerWidget();

	void Update();
	void Clear();

	inline CEntity* SelectedEntity() const { return selectedEnt; }

Q_SIGNALS:
	void entitySelected(CEntity*);

private:
	TMap<TObjectPtr<CEntity>, TTreeDataItem<CEntity*>*> entityItems;

	CEntity* selectedEnt;
	QLineEdit* filter;
	QTreeWidget* outlinerTree;

};
