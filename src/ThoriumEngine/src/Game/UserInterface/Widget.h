#pragma once

#include "Canvas.h"
#include "Widget.generated.h"

CLASS()
class CWidget : public CObject
{
	GENERATED_BODY()

public:
	CWidget(CWidget* parent = nullptr);
	explicit CWidget(CCanvas* parent);

	virtual void Render(CRenderScene* scene, FVector2 screenSize);

	virtual void KeyEvent(CKeyEvent* event);
	virtual void MouseEvent(CMouseEvent* event);
	virtual void CharEvent(wchar_t);

	void CalculateScreenTransform();

	inline const FVector2& GetScreenPos() const { return screenPos; }
	inline const FVector2& GetScreenScale() const { return screenScale; }

	inline bool MouseTracking() const { return bMouseTracking; }

	void SetFocus();

protected:
	virtual void OnDelete();

public:
	PROPERTY(Editable)
	int zOrder;

	PROPERTY(Editable)
	FVector2 position;
	PROPERTY(Editable)
	FVector2 scale;

	PROPERTY(Editable)
	FVector2 anchorMin;
	PROPERTY(Editable)
	FVector2 anchorMax;

	PROPERTY(Editable)
	FVector2 pivot;

protected:
	bool bMouseTracking = false;

private:
	TObjectPtr<CWidget> parent;
	TObjectPtr<CCanvas> canvas;
	TArray<TObjectPtr<CWidget>> childWidgets;

	FVector2 screenPos;
	FVector2 screenScale;

};
