#pragma once

#include <QWidget>
#include <QFrame>
#include <Util/Event.h>
#include <chrono>

class CRenderScene;
class ISwapChain;
class CRenderWidgetWnd;
class IBaseWindow;

class CRenderWidget : public QFrame
{
	Q_OBJECT

public:
	CRenderWidget(QWidget* parent);
	virtual ~CRenderWidget();

	inline CRenderScene* GetRenderScene() const { return scene; }

	inline CRenderScene* GetOverrideScene() const { return overrideScene; }
	inline void SetOverrideScene(CRenderScene* scene) { overrideScene = scene; }

	//inline TEvent<>& GetRenderEvent() { return onRender; }

	void Render();

	inline double GetDeltaTime() { return dt; }

	inline IBaseWindow* GetWindow() const { return (IBaseWindow*)_window; }

protected:
	virtual void Update(double dt) {}

	QPaintEngine* paintEngine() const { return nullptr; }
	void paintEvent(QPaintEvent *event) override {}

	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void resizeEvent(QResizeEvent *event) override;

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

protected:
	std::chrono::time_point<std::chrono::steady_clock> lastRenderTime;
	double dt = 0.f;
	CRenderScene* scene;
	CRenderScene* overrideScene;
	ISwapChain* swapchain;
	CRenderWidgetWnd* _window;

};
