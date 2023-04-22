#pragma once

#include "AdvancedDockWidgets/DockWidget.h"
#include "EngineCore.h"
#include "ToolsCore.h"
#include "Widgets/CollapsableWidget.h"

class QVBoxLayout;
class QScrollArea;

class SDK_API CSceneSettingsWidget : public ads::CDockWidget
{
	Q_OBJECT

public:
	CSceneSettingsWidget(QWidget* parent = nullptr);

	void LevelChanged();

	void SetupUI();
	void ClearUI();

	CCollapsableWidget* GetHeader(const QString& name);

private:
	QWidget* widget;
	QScrollArea* scroll;
	QVBoxLayout* scrollLayout;
	TArray<CCollapsableWidget*> headers;

};
