
#include "RenderWidget.h"
#include "Window.h"
#include "Rendering/Framebuffer.h"
#include "Rendering/Renderer.h"
#include "Rendering/RenderScene.h"
#include <QResizeEvent>

class CRenderWidgetWnd : public IBaseWindow
{
public:
	CRenderWidgetWnd(void* wnd, int w, int h) : wndHandle(wnd) {
		width = w;
		height = h;
	}

	virtual void* GetNativeHandle() { return wndHandle; }

protected:
	void* wndHandle;
};

CRenderWidget::CRenderWidget(QWidget* parent) : QFrame(parent)
{
	HWND wnd = reinterpret_cast<HWND>(winId());
	CRenderWidgetWnd _window(wnd, size().width(), size().height());

	setMinimumSize({ 128, 128 });
	setAutoFillBackground(false);
	setUpdatesEnabled(false);

	swapchain = gRenderer->CreateSwapChain(&_window);
	scene = new CRenderScene();
}

CRenderWidget::~CRenderWidget()
{
	delete scene;
	delete swapchain;
}

void CRenderWidget::Render()
{
	Update(dt);

	scene->SetFrameBuffer(swapchain->GetFrameBuffer());
	scene->SetDepthBuffer(swapchain->GetDepthBuffer());

	gRenderer->PushScene(scene);
	gRenderer->Render();

	swapchain->Present(0, 0);

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

void CRenderWidget::resizeEvent(QResizeEvent *event)
{
	swapchain->Resize(event->size().width(), event->size().height());
}
