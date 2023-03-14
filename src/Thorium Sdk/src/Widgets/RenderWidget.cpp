
#include "RenderWidget.h"
#include "Window.h"
#include "Rendering/Framebuffer.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderScene.h"
#include <QResizeEvent>
#include <QKeyEvent>

class CRenderWidgetWnd : public IBaseWindow
{
public:
	CRenderWidgetWnd(void* wnd, int w, int h) : wndHandle(wnd) {
		width = w;
		height = h;
	}

	virtual void* GetNativeHandle() { return wndHandle; }

	void SetSize(int w, int h) { width = w; height = h; }
	void SetMousePos(double x, double y) { mouseX = x; mouseY = y; }

protected:
	void* wndHandle;
};

EMouseButton ConvertMouseBtn(Qt::MouseButton btn)
{
	switch (btn)
	{
	case Qt::LeftButton:
		return EMouseButton::LEFT;
	case Qt::RightButton:
		return EMouseButton::RIGHT;
	case Qt::MiddleButton:
		return EMouseButton::MIDDLE;
	case Qt::BackButton:
		return EMouseButton::MOUSE4;
	case Qt::ForwardButton:
		return EMouseButton::MOUSE5;
	case Qt::ExtraButton3:
		return EMouseButton::MOUSE6;
	case Qt::ExtraButton4:
		return EMouseButton::MOUSE7;
	case Qt::ExtraButton5:
		return EMouseButton::MOUSE8;
	}
	return EMouseButton::NONE;
}

EKeyCode ConvertKey(Qt::Key key)
{
	switch (key)
	{
	case Qt::Key_Escape:
		return EKeyCode::ESCAPE;
	case Qt::Key_Tab:
		return EKeyCode::TAB;
	case Qt::Key_Backspace:
		return EKeyCode::BACKSPACE;
	case Qt::Key_Enter:
		return EKeyCode::ENTER;
	case Qt::Key_Insert:
		return EKeyCode::INSERT;
	case Qt::Key_Delete:
		return EKeyCode::KEY_DELETE;
	case Qt::Key_Pause:
		return EKeyCode::PAUSE;
	case Qt::Key_Print:
		return EKeyCode::PRINT_SCREEN;
	case Qt::Key_Home:
		return EKeyCode::HOME;
	case Qt::Key_End:
		return EKeyCode::END;
	case Qt::Key_Left:
		return EKeyCode::LEFT;
	case Qt::Key_Up:
		return EKeyCode::UP;
	case Qt::Key_Down:
		return EKeyCode::DOWN;
	case Qt::Key_Right:
		return EKeyCode::RIGHT;
	case Qt::Key_PageUp:
		return EKeyCode::PAGE_UP;
	case Qt::Key_PageDown:
		return EKeyCode::PAGE_DOWN;
	case Qt::Key_Shift:
		return EKeyCode::LEFT_SHIFT;
	case Qt::Key_Control:
		return EKeyCode::LEFT_CONTROL;
	case Qt::Key_Alt:
		return EKeyCode::LEFT_ALT;
	case Qt::Key_CapsLock:
		return EKeyCode::CAPS_LOCK;
	case Qt::Key_NumLock:
		return EKeyCode::NUM_LOCK;
	case Qt::Key_ScrollLock:
		return EKeyCode::SCROLL_LOCK;
	case Qt::Key_F1:
		return EKeyCode::F1;
	case Qt::Key_F2:
		return EKeyCode::F2;
	case Qt::Key_F3:
		return EKeyCode::F3;
	case Qt::Key_F4:
		return EKeyCode::F4;
	case Qt::Key_F5:
		return EKeyCode::F5;
	case Qt::Key_F6:
		return EKeyCode::F6;
	case Qt::Key_F7:
		return EKeyCode::F7;
	case Qt::Key_F8:
		return EKeyCode::F8;
	case Qt::Key_F9:
		return EKeyCode::F9;
	case Qt::Key_F10:
		return EKeyCode::F10;
	case Qt::Key_F11:
		return EKeyCode::F11;
	case Qt::Key_F12:
		return EKeyCode::F12;
	case Qt::Key_F13:
		return EKeyCode::F13;
	case Qt::Key_F14:
		return EKeyCode::F14;
	case Qt::Key_F15:
		return EKeyCode::F15;
	case Qt::Key_F16:
		return EKeyCode::F16;
	case Qt::Key_F17:
		return EKeyCode::F17;
	case Qt::Key_F18:
		return EKeyCode::F18;
	case Qt::Key_F19:
		return EKeyCode::F19;
	case Qt::Key_F20:
		return EKeyCode::F20;
	case Qt::Key_F21:
		return EKeyCode::F21;
	case Qt::Key_F22:
		return EKeyCode::F22;
	case Qt::Key_F23:
		return EKeyCode::F23;
	case Qt::Key_F24:
		return EKeyCode::F24;
	case Qt::Key_F25:
		return EKeyCode::F25;
	case Qt::Key_Menu:
		return EKeyCode::MENU;
	case Qt::Key_Space:
		return EKeyCode::SPACE;
	case Qt::Key_Equal:
		return EKeyCode::EQUAL;
	case Qt::Key_Minus:
		return EKeyCode::MINUS;
	case Qt::Key_Apostrophe:
		return EKeyCode::APOSTROPHE;
	case Qt::Key_Comma:
		return EKeyCode::COMMA;
	case Qt::Key_Period:
		return EKeyCode::PERIOD;
	case Qt::Key_Slash:
		return EKeyCode::SLASH;
	case Qt::Key_0:
		return EKeyCode::KEY_0;
	case Qt::Key_1:
		return EKeyCode::KEY_1;
	case Qt::Key_2:
		return EKeyCode::KEY_2;
	case Qt::Key_3:
		return EKeyCode::KEY_3;
	case Qt::Key_4:
		return EKeyCode::KEY_4;
	case Qt::Key_5:
		return EKeyCode::KEY_5;
	case Qt::Key_6:
		return EKeyCode::KEY_6;
	case Qt::Key_7:
		return EKeyCode::KEY_7;
	case Qt::Key_8:
		return EKeyCode::KEY_8;
	case Qt::Key_9:
		return EKeyCode::KEY_9;
	case Qt::Key_Semicolon:
		return EKeyCode::SEMICOLON;
	case Qt::Key_A:
		return EKeyCode::A;
	case Qt::Key_B:
		return EKeyCode::B;
	case Qt::Key_C:
		return EKeyCode::C;
	case Qt::Key_D:
		return EKeyCode::D;
	case Qt::Key_E:
		return EKeyCode::E;
	case Qt::Key_F:
		return EKeyCode::F;
	case Qt::Key_G:
		return EKeyCode::G;
	case Qt::Key_H:
		return EKeyCode::H;
	case Qt::Key_I:
		return EKeyCode::I;
	case Qt::Key_J:
		return EKeyCode::J;
	case Qt::Key_K:
		return EKeyCode::K;
	case Qt::Key_L:
		return EKeyCode::L;
	case Qt::Key_M:
		return EKeyCode::M;
	case Qt::Key_N:
		return EKeyCode::N;
	case Qt::Key_O:
		return EKeyCode::O;
	case Qt::Key_P:
		return EKeyCode::P;
	case Qt::Key_Q:
		return EKeyCode::Q;
	case Qt::Key_R:
		return EKeyCode::R;
	case Qt::Key_S:
		return EKeyCode::S;
	case Qt::Key_T:
		return EKeyCode::T;
	case Qt::Key_U:
		return EKeyCode::U;
	case Qt::Key_V:
		return EKeyCode::V;
	case Qt::Key_W:
		return EKeyCode::W;
	case Qt::Key_X:
		return EKeyCode::X;
	case Qt::Key_Y:
		return EKeyCode::Y;
	case Qt::Key_Z:
		return EKeyCode::Z;
	case Qt::Key_Backslash:
		return EKeyCode::BACKSLASH;
	case Qt::Key_BracketLeft:
		return EKeyCode::LEFT_BRACKET;
	case Qt::Key_BracketRight:
		return EKeyCode::RIGHT_BRACKET;
	}

	return EKeyCode::UNKOWN;
}

EInputMod ConvertMod(Qt::KeyboardModifiers mod)
{
	int m = IM_NONE;
	if (mod & Qt::ShiftModifier)
		m |= IM_SHIFT;
	if (mod & Qt::ControlModifier)
		m |= IM_CONTROL;
	if (mod & Qt::AltModifier)
		m |= IM_ALT;
	if (mod & Qt::KeypadModifier)
		m |= IM_NUM_LOCK;

	return (EInputMod)m;
}

CRenderWidget::CRenderWidget(QWidget* parent) : QFrame(parent)
{
	HWND wnd = reinterpret_cast<HWND>(winId());
	_window = new CRenderWidgetWnd(wnd, size().width(), size().height());

	setMinimumSize({ 128, 128 });
	setAutoFillBackground(false);
	setUpdatesEnabled(false);

	swapchain = gRenderer->CreateSwapChain(_window);
	scene = new CRenderScene();
	overrideScene = nullptr;
}

CRenderWidget::~CRenderWidget()
{
	delete _window;
	delete scene;
	delete swapchain;
}

void CRenderWidget::Render()
{
	Update(dt);

	CRenderScene* targetScene = overrideScene ? overrideScene : scene;

	targetScene->SetFrameBuffer(swapchain->GetFrameBuffer());
	targetScene->SetDepthBuffer(swapchain->GetDepthBuffer());

	gRenderer->PushScene(targetScene);
	gRenderer->Render();

	swapchain->Present(1, 0);

	// Update D.T.
	{
		using std::chrono::high_resolution_clock;
		using std::chrono::duration_cast;
		using std::chrono::duration;
		using std::chrono::milliseconds;

		auto tNow = high_resolution_clock::now();
		auto tc = duration_cast<milliseconds>(tNow - lastRenderTime);
		lastRenderTime = tNow;

		dt = ((double)tc.count() / 1000.0);
	}
}

void CRenderWidget::mousePressEvent(QMouseEvent* event)
{
	// TODO: fix button type.
	_window->OnMouseButton.Fire(ConvertMouseBtn(event->button()), IE_PRESS, ConvertMod(event->modifiers()));
	QFrame::mousePressEvent(event);
}

void CRenderWidget::mouseReleaseEvent(QMouseEvent* event)
{
	_window->OnMouseButton.Fire(ConvertMouseBtn(event->button()), IE_RELEASE, ConvertMod(event->modifiers()));
	QFrame::mouseReleaseEvent(event);
}

void CRenderWidget::mouseMoveEvent(QMouseEvent* event)
{
	auto p = event->localPos();
	_window->SetMousePos(p.x(), p.y());
	QFrame::mouseMoveEvent(event);
}

void CRenderWidget::resizeEvent(QResizeEvent *event)
{
	_window->SetSize(event->size().width(), event->size().height());
	swapchain->Resize(event->size().width(), event->size().height());
	QFrame::resizeEvent(event);
}

void CRenderWidget::keyPressEvent(QKeyEvent* event)
{
	_window->OnKeyEvent.Fire(ConvertKey((Qt::Key)event->key()), IE_PRESS, ConvertMod(event->modifiers()));
	QFrame::keyPressEvent(event);
}

void CRenderWidget::keyReleaseEvent(QKeyEvent* event)
{
	_window->OnKeyEvent.Fire(ConvertKey((Qt::Key)event->key()), IE_RELEASE, ConvertMod(event->modifiers()));
	QFrame::keyReleaseEvent(event);
}
