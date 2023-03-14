
#include "OutlinerWidget.h"
#include "Widgets/TreeDataItem.h"
#include "Game/World.h"
#include "Game/Entity.h"
#include <Util/Map.h>

#include <QLineEdit>
#include <QTreeWidget>
#include <QBoxLayout>
#include <QDropEvent>

class COutlinerTreeWidget : public QTreeWidget
{
public:
	COutlinerTreeWidget(QWidget* parent = nullptr) : QTreeWidget(parent) {}

protected:
	void dragEnterEvent(QDragEnterEvent* event);
	void dropEvent(QDropEvent* event);

private:
	TTreeDataItem<CEntity*>* draggedItem = nullptr;

};

void COutlinerTreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
	draggedItem = (TTreeDataItem<CEntity*>*)currentItem();
	QTreeWidget::dragEnterEvent(event);
}

void COutlinerTreeWidget::dropEvent(QDropEvent* event)
{
	QModelIndex index = indexAt(event->pos());
	if (!index.isValid())
	{
		if (draggedItem)
		{
			draggedItem->GetData()->SetOwner(nullptr);
			draggedItem->GetData()->RootComponent()->Detach();
		}
		return;
	}

	if (draggedItem)
	{
		TTreeDataItem<CEntity*>* targetItem = (TTreeDataItem<CEntity*>*)itemFromIndex(index);
		if (targetItem)
		{
			draggedItem->GetData()->SetOwner(targetItem->GetData());
			draggedItem->GetData()->RootComponent()->AttachTo(targetItem->GetData()->RootComponent());
		}
	}
}

COutlinerWidget::COutlinerWidget(QWidget* parent /*= nullptr*/) : QDockWidget(parent)
{
	QWidget* pWidget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout(pWidget);
	layout->setContentsMargins(4, 4, 4, 4);

	setWidget(pWidget);

	setObjectName("outliner_widget");
	setWindowTitle("Outliner");

	filter = new QLineEdit(this);
	filter->setPlaceholderText("Search...");
	filter->setMinimumHeight(24);
	filter->setStyleSheet("QLineEdit { border-radius: 10px; }");

	outlinerTree = new COutlinerTreeWidget(this);
	outlinerTree->setSelectionBehavior(QAbstractItemView::SelectRows);
	outlinerTree->setDragDropMode(QAbstractItemView::InternalMove);
	outlinerTree->setDragEnabled(true);
	//outlinerTree->setAcceptDrops(true);

	outlinerTree->setColumnCount(2);
	outlinerTree->setHeaderLabels({ "Name", "Type" });

	layout->addWidget(filter);
	layout->addWidget(outlinerTree);

	connect(outlinerTree, &QTreeWidget::currentItemChanged, this, [=](QTreeWidgetItem* current, QTreeWidgetItem*) { 
		if (current)
		{
			selectedEnt = ((TTreeDataItem<CEntity*>*)current)->GetData();
			emit(entitySelected(selectedEnt));
		}
		else
			selectedEnt = nullptr;
	});
}

COutlinerWidget::~COutlinerWidget()
{
	filter->deleteLater();
	outlinerTree->deleteLater();
}

void COutlinerWidget::Update()
{
	auto& ents = gWorld->GetEntities();

	outlinerTree->blockSignals(true);
	for (auto it = entityItems.rbegin(); it != entityItems.rend(); it++)
	{
		if (it->first && ents.Find(it->first) == ents.end())
		{
			if (it->second->parent())
				it->second->parent()->removeChild(it->second);
			else
				outlinerTree->invisibleRootItem()->removeChild(it->second);
			entityItems.erase(it->first);
		}
	}
	outlinerTree->blockSignals(false);

	for (auto& ent : ents)
	{
		if (ent->bEditorEntity)
			continue;

		TTreeDataItem<CEntity*>* entItem = entityItems[ent];
		if (!entItem)
		{
			entItem = new TTreeDataItem<CEntity*>(ent, entityItems[ent->GetOwner<CEntity>()]);
			entItem->setText(0, ent->Name().c_str());
			entItem->setText(1, ent->GetClass()->GetName().c_str());

			entityItems[ent] = entItem;
			if (ent->GetOwner() == nullptr)
				outlinerTree->addTopLevelItem(entItem);
		}
		else
		{
			auto* parentItem = entItem->parent();
			if (entItem->text(0) != ent->Name().c_str())
				entItem->setText(0, ent->Name().c_str());

			if (!ent->GetOwner() && parentItem)
			{
				parentItem->removeChild(entItem);
				outlinerTree->addTopLevelItem(entItem);
			}
			else if (CEntity* owner = ent->GetOwner<CEntity>())
			{
				auto newParent = entityItems[owner];
				if (newParent && newParent != parentItem)
				{
					if (parentItem)
						parentItem->removeChild(entItem);
					else
						outlinerTree->invisibleRootItem()->removeChild(entItem);
					newParent->addChild(entItem);
				}
			}
		}
	}
}

void COutlinerWidget::Clear()
{
	outlinerTree->blockSignals(true);
	for (auto it = entityItems.rbegin(); it != entityItems.rend(); it++)
	{
		if (!it->first)
			return;

		if (it->second->parent())
			it->second->parent()->removeChild(it->second);
		else
			outlinerTree->invisibleRootItem()->removeChild(it->second);
	}
	entityItems.clear();
	outlinerTree->blockSignals(false);
}
