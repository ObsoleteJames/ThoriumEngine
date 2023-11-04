
#include "RenderWidget.h"
#include "Math/Vectors.h"
#include <QPoint>

class CCameraComponent;
class CCameraProxy;

enum class ECameraControlMode
{
	Disabled,
	FreeMode,
	Orbit
};

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

	inline FVector GetMoveVector() const { return FVector(moveLeft + -moveRight, -moveForward + moveBack, 0); }

public:
	//void SetCamera(CCameraComponent* cam);
	void SetCamera(CCameraProxy* cam);
	inline void SetControlMode(ECameraControlMode m) { mode = m; }

public:
	//CCameraComponent* camera;
	CCameraProxy* camera;
	int cameraMode = 0;


private:
	ECameraControlMode mode = ECameraControlMode::FreeMode;

	QPoint mouseClickPos;
	int cameraSpeed = 4;
	float cameraPitch = 0;
	float cameraYaw = 0;
	FVector orbitPos;

	float curSpeed = 0.f;

	int prevMouseX = 0;
	int prevMouseY = 0;

	int8 moveForward : 1;
	int8 moveBack : 1;
	int8 moveLeft : 1;
	int8 moveRight : 1;
	int8 moveUp : 1;
	int8 moveDown : 1;

	float mouseDeltaX;
	float mouseDeltaY;

	bool bMouseLeft : 1;
	bool bMouseRight : 1;
	bool bMouseMiddle : 1;

};
