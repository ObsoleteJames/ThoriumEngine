
#include "Canvas.h"

CWidget* CCanvas::focusWidget;

CCanvas::CCanvas()
{
}

void CCanvas::Update(float dt)
{

}

void CCanvas::Render(CRenderScene* scene)
{

}

void CCanvas::AddWidget(CWidget* widget)
{
	widgets.Add(widget);
}

void CCanvas::RemoveWidget(CWidget* widget)
{
	if (auto it = widgets.Find(widget); it != widgets.end())
		widgets.Erase(it);
}

void CCanvas::SetSize(const FVector2& size)
{
	if (scaleMode == CSM_ConstantSize)
		this->size = size;
}

void CCanvas::SetFocusTo(CWidget* widget)
{
	focusWidget = widget;
}

void CCanvas::KeyEvent(CKeyEvent* event)
{

}

void CCanvas::MouseEvent(CMouseEvent* event)
{

}
