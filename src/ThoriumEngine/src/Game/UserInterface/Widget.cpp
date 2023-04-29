
#include "Widget.h"

CWidget::CWidget(CWidget* p /*= nullptr*/)
{
	parent = p;
	canvas = parent->canvas;
	p->childWidgets.Add(this);
}

CWidget::CWidget(CCanvas* p)
{
	parent = nullptr;
	canvas = p;
	p->AddWidget(this);
}

void CWidget::Render(CRenderScene* scene, FVector2 screenSize)
{
}

void CWidget::KeyEvent(CKeyEvent* event)
{
}

void CWidget::MouseEvent(CMouseEvent* event)
{
}

void CWidget::CharEvent(wchar_t)
{
}

void CWidget::CalculateScreenTransform()
{
	FVector2 parentPos{};
	FVector2 parentSize{};

	if (parent)
	{
		parentPos = parent->GetScreenPos();
		parentSize = parent->GetScreenScale();
	}
	else if (canvas)
		parentSize = canvas->GetSize();

	screenPos = position + parentPos;
	screenPos.x += parentSize.x * anchorMin.x;
	screenPos.y += parentSize.y * anchorMin.y;

	screenScale = scale;
	screenScale.x += (parentSize.x * anchorMax.x) - (parentSize.x * anchorMin.x);
	screenScale.y += (parentSize.y * anchorMax.y) - (parentSize.y * anchorMin.y);

	for (auto& w : childWidgets)
		w->CalculateScreenTransform();
}

void CWidget::SetFocus()
{
	if (canvas) 
		canvas->SetFocusTo(this);
}

void CWidget::OnDelete()
{
	if (parent)
	{
		parent->childWidgets.Erase(parent->childWidgets.Find(this));
	}
	else if (canvas)
	{
		canvas->RemoveWidget(this);
	}
}
