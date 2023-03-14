
#include "RenderWidget.h"
#include <QPoint>

class CCameraComponent;

class CWorldViewportWidget : public CRenderWidget
{
	Q_OBJECT

public:
	CWorldViewportWidget(QWidget* parent);
	virtual ~CWorldViewportWidget();

protected:
	void Update(double dt) override;

	void mouseMoveEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;

	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;

	void wheelEvent(QWheelEvent *event);

public:
	CCameraComponent* camera;
	int cameraMode = 0;

private:
	QPoint mouseClickPos;
	int cameraSpeed = 4;
	float cameraPitch = 0;
	float cameraYaw = 0;

	int prevMouseX = 0;
	int prevMouseY = 0;

	int moveInX = 0;
	int moveInY = 0;

	float mouseDeltaX;
	float mouseDeltaY;

	bool bMouseLeft : 1;
	bool bMouseRight : 1;
	bool bMouseMiddle : 1;

};
