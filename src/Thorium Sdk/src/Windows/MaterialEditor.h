#pragma once

#include "ToolsWindow.h"
#include "Object/Object.h"

class CObjectPtrProperty;
class CCollapsableWidget;
class CWorldViewportWidget;
class CWorld;
class CMaterial;
class CModelComponent;
class CCameraProxy;
class QVBoxLayout;

class CMaterialEditor : public CToolsWindow
{
	Q_OBJECT
	ToolsWindowBody(CMaterialEditor, "Material Editor", true)

public:
	CMaterialEditor() = default;

	bool Shutdown() override;
	void SetupUi() override;

	void Init();

protected:
	void paintEvent(QPaintEvent* event) override;
	void closeEvent(QCloseEvent* event) override;

private:
	bool ShutdownSave();

	void SetMaterial(CMaterial* mat, bool bNew = false);
	void SaveMaterial(bool bNew = false);

	void NewMaterial();
	void LoadMaterial();

	void UpdateUI();
	void UpdateTitle();

	CCollapsableWidget* GetHeader(const FString& category);

private:
	bool bRequiresSave = false;
	bool bReadOnly = false;

	TObjectPtr<CWorld> world;
	TObjectPtr<CMaterial> material;
	TObjectPtr<CModelComponent> modelComp;

	CCameraProxy* cam;

	TArray<CCollapsableWidget*> curWidgets;

	CObjectPtrProperty* shaderEditor;

	QDockWidget* editorWidget;
	CWorldViewportWidget* viewport;
	QVBoxLayout* contentLayout;

};
