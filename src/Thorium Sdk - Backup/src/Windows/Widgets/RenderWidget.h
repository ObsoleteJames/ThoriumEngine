#pragma once

#include <QWidget>
#include <QFrame>
#include <Util/Event.h>
#include <chrono>

class CRenderScene;
class ISwapChain;

class CRenderWidget : public QFrame
{
	Q_OBJECT

public:
	CRenderWidget(QWidget* parent);
	virtual ~CRenderWidget();

	inline CRenderScene* GetRenderScene() const { return scene; }
	//inline TEvent<>& GetRenderEvent() { return onRender; }

	void Render();

	inline double GetDeltaTime() { return dt; }

protected:
	virtual void Update(double dt) {}

	QPaintEngine* paintEngine() const { return nullptr; }
	void paintEvent(QPaintEvent *event) override {}

	void resizeEvent(QResizeEvent *event) override;

protected:
	std::chrono::time_point<std::chrono::steady_clock> lastRenderTime;
	double dt = 0.f;
	CRenderScene* scene;
	ISwapChain* swapchain;

};
