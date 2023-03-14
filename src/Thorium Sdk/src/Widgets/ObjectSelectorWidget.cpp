
#include <string>
#include "ObjectSelectorWidget.h"
#include "Resources/Asset.h"

#include <QDialog>
#include <QListWidget>
#include <QStylePainter>
#include <QStyleOption>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QLineEdit>

class CObjectSelectDialog : public QDialog
{
public:
	CObjectSelectDialog(CObjectSelectorWidget* obj) : QDialog(obj), objectWidget(obj)
	{
		setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
		setMaximumHeight(500);
		auto* layout = new QVBoxLayout(this);

		searchEdit = new QLineEdit(this);
		searchEdit->setPlaceholderText("Search...");

		list = new QListWidget(this);
		
		layout->addWidget(searchEdit);
		layout->addWidget(list);

		PopulateList();

		QPoint p = objectWidget->mapToGlobal(QPoint()) + QPoint(0, objectWidget->height());
		QRect rect(p, QSize(objectWidget->width(), 360));
		setGeometry(rect);

		//connect(list, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
		//	if (objectWidget->bTypeIsAsset)
		//	{
		//		this->obj = (CObject*)item->data(257).toULongLong();
		//	}
		//	else
		//	{

		//	}
		//});
		connect(searchEdit, &QLineEdit::textChanged, this, [=](const QString& txt) {
			PopulateList(txt);
		});
		connect(list, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem* item) {
			if (objectWidget->bTypeIsAsset)
			{
				QString path = item->data(257).toString();
				if (!path.isEmpty())
				{
					WString p = path.toStdWString();
					this->obj = CResourceManager::GetResource((FAssetClass*)objectWidget->GetClassFilter(), p);
				}
				else
					this->obj = nullptr;
			}
			else
				this->obj = (CObject*)item->data(257).toULongLong();

			done(true); 
		});
	}

	void PopulateList(const QString& filter = QString())
	{
		list->clear();
		if (objectWidget->bAllowNull)
			list->addItem("None");

		if (!objectWidget->bTypeIsAsset)
		{
			const TMap<FGuid, CObject*>& objects = CObjectManager::GetAllObjects();
			for (auto& it : objects)
			{
				if (it.second->GetClass()->CanCast(objectWidget->GetClassFilter()))
				{
					if (objectWidget->validator && !objectWidget->validator(it.second))
						continue;

					QString text = it.second->Name().c_str();
					if (!filter.isEmpty() && !text.contains(filter))
						continue;

					auto* item = new QListWidgetItem(text, list);
					item->setData(257, (uint64)it.second);
				}
			}
		}
		else
		{
			auto& assets = CResourceManager::GetAvailableResources();
			for (auto& it : assets)
			{
				if (it.second.type == objectWidget->GetClassFilter())
				{
					if (objectWidget->assetValidator && !objectWidget->assetValidator(it.first))
						continue;

					QString text = QString((const QChar*)it.second.file->Name().c_str());
					if (!filter.isEmpty() && !text.contains(filter))
						continue;

					auto* item = new QListWidgetItem(text, list);
					item->setData(257, QString((const QChar*)it.first.c_str()));
				}
			}
		}
	}

	inline CObject* GetObject() const { return obj; }

private:
	CObject* obj;
	CObjectSelectorWidget* objectWidget;
	QLineEdit* searchEdit;
	QListWidget* list;

};

CObjectSelectorWidget::CObjectSelectorWidget(QWidget* parent /*= nullptr*/) : QWidget(parent)
{
	Init();
}

CObjectSelectorWidget::CObjectSelectorWidget(FClass* filter, QWidget* parent /*= nullptr*/) : QWidget(parent), filterType(filter)
{
	Init();
}

CObjectSelectorWidget::CObjectSelectorWidget(CObject* obj, FClass* filter, QWidget* parent /*= nullptr*/) : QWidget(parent), filterType(filter)
{
	Init();
	SetObject(obj);
}

void CObjectSelectorWidget::Init()
{
	setMaximumHeight(24);
	setMinimumHeight(18);
	
	validator = &CObjectSelectorWidget::DefaultValidator;

	if (filterType)
		bTypeIsAsset = filterType->CanCast(CAsset::StaticClass());
}

void CObjectSelectorWidget::SetObject(CObject* obj)
{
	this->obj = obj;
	update();
}

void CObjectSelectorWidget::SetClassFilter(FClass* type)
{
	filterType = type;
	bTypeIsAsset = filterType->CanCast(CAsset::StaticClass());

	if (obj && !obj->GetClass()->CanCast(filterType))
	{
		obj = nullptr;
		update();
	}
}

void CObjectSelectorWidget::paintEvent(QPaintEvent* event)
{
	QStylePainter painter(this);
	
	QString text;
	if (obj)
	{
		if (bTypeIsAsset)
		{
			TObjectPtr<CAsset> asset = Cast<CAsset>(obj);
			if (asset->File())
				text = QString((const QChar*)asset->File()->Name().c_str());
			else
				text = obj->Name().c_str();
		}
		else
			text = obj->Name().c_str();
	}

	QStyleOptionButton opt;
	opt.initFrom(this);
	opt.features |= QStyleOptionButton::HasMenu;
	opt.text = text;
	opt.direction = Qt::LeftToRight;
	opt.rect = rect();
	QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);

	painter.drawControl(QStyle::CE_PushButton, opt);

	if (!obj)
	{
		const Qt::LayoutDirection layoutDir = opt.direction;
		QPen oldPen = painter.pen();
		painter.setPen(QColor(127, 127, 127));

		painter.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, "None");
		painter.setPen(oldPen);
	}
}

void CObjectSelectorWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() != Qt::LeftButton)
		return;

	CObjectSelectDialog dialog(this);
	if (dialog.exec())
	{
		obj = dialog.GetObject();
		emit(ObjectChanged());
		update();
	}
}
