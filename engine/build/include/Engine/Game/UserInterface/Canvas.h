#pragma once

#include "Object/Object.h"
#include "Math/Vectors.h"
#include "Canvas.generated.h"

class CWidget;
class CRenderScene;
class CKeyEvent;
class CMouseEvent;

ENUM()
enum ECanvasScaleMode
{
	CSM_ScaleToScreen,
	CSM_ConstantSize,
};

CLASS()
class CCanvas : public CObject
{
	GENERATED_BODY()

	friend class CInputManager;

public:
	CCanvas();

	void Update(float dt);
	void Render(CRenderScene* scene);

	void AddWidget(CWidget* widget);
	void RemoveWidget(CWidget* widget);

	inline const FVector2& GetSize() const { return size; }
	void SetSize(const FVector2& size);

	void SetFocusTo(CWidget* widget);
	inline CWidget* FocusedWidget() const { return focusWidget; }

	inline int ZOrder() const { return zOrder; }
	void SetZOrder(int z) { zOrder = z; }

protected:
	void KeyEvent(CKeyEvent* event);
	void MouseEvent(CMouseEvent* event);

public:
	PROPERTY(Editable)
	bool bEnabled = true;

	PROPERTY(Editable)
	ECanvasScaleMode scaleMode = CSM_ScaleToScreen;

private:
	// Top Level widgets.
	TArray<TObjectPtr<CWidget>> widgets;

	// Widget that has focus.
	static CWidget* focusWidget;

	int zOrder;

	FVector2 size = { 1920, 1080 };

};
