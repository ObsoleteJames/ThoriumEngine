
#include "SceneSettingsWidget.h"
#include "Game/World.h"
#include "Game/GameMode.h"
#include "Resources/Scene.h"

#include "Widgets/PropertyEditors/ClassPtrProperty.h"

#include <QScrollArea>
#include <QBoxLayout>

CSceneSettingsWidget::CSceneSettingsWidget(QWidget* parent /*= nullptr*/) : ads::CDockWidget("Scene Settings", parent)
{
	widget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout(widget);
	layout->setMargin(0);
	layout->setSpacing(0);

	setWidget(widget);

	setObjectName("scene_settings_widget");

	scroll = new QScrollArea(this);
	scroll->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	scroll->setWidgetResizable(true);
	scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QFrame* content = new QFrame(this);
	scrollLayout = new QVBoxLayout(content);
	content->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
	scroll->setWidget(content);
	scrollLayout->setContentsMargins(0, 0, 0, 0);
	scrollLayout->setSpacing(0);

	layout->addWidget(scroll);

	if (gWorld && gWorld->GetScene())
	{
		SetupUI();
	}
}

void CSceneSettingsWidget::LevelChanged()
{
	ClearUI();
	if (gWorld && gWorld->GetScene())
	{
		SetupUI();
	}
}

void CSceneSettingsWidget::SetupUI()
{
	CScene* scene = gWorld->GetScene();

	auto* gmHeader = GetHeader("Gamemode");
	gmHeader->SetCollapsed(false);

	auto* gmEdit = new CClassPtrProperty("Gamemode", &scene->gamemodeClass, CGameMode::StaticClass(), gmHeader->Widget());

	gmHeader->Widget()->layout()->addWidget(gmEdit);
}

void CSceneSettingsWidget::ClearUI()
{
	for (auto* h : headers)
	{
		scrollLayout->removeWidget(h);
		delete h;
	}
	headers.Clear();
}

CCollapsableWidget* CSceneSettingsWidget::GetHeader(const QString& name)
{
	for (auto* h : headers)
		if (h->Text() == name)
			return h;

	QWidget* w = new QWidget(this);
	QVBoxLayout* l = new QVBoxLayout(w);
	l->setMargin(0);
	l->setSpacing(0);

	auto* h = new CCollapsableWidget(name, w, this);
	headers.Add(h);
	scrollLayout->addWidget(h);
	return h;
}
