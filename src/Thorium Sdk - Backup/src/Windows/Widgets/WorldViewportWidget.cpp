
#include <string>
#include "EditorEngine.h"
#include "WorldViewportWidget.h"
#include "Game/Components/CameraComponent.h"
#include "Math/Vectors.h"
#include "Console.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QApplication>

float GetCameraSpeed(int index)
{
	float speed = 0.5f;
	speed *= (float)index;
	speed *= speed;
	return speed;
}

CWorldViewportWidget::CWorldViewportWidget(QWidget* parent) : CRenderWidget(parent), bMouseLeft(0), bMouseMiddle(0), bMouseRight(0)
{
	setMouseTracking(true);
	setFocusPolicy(Qt::StrongFocus);
	bMouseLeft = 0;
	bMouseMiddle = 0;
	bMouseRight = 0;
}

CWorldViewportWidget::~CWorldViewportWidget()
{
}

void CWorldViewportWidget::Update(double dt)
{
	if (bMouseRight)
	{
		cameraPitch = FMath::Clamp(cameraPitch + mouseDeltaY, -90.f, 90.f);
		cameraYaw += mouseDeltaX;
		camera->SetRotation(FQuaternion::EulerAngles(FVector(cameraPitch, cameraYaw, 0.f).Radians()));

		if (moveInY != 0 || moveInX != 0)
		{
			FVector pos = camera->GetPosition();
			pos += camera->GetForwardVector() * moveInY * GetCameraSpeed(cameraSpeed) * dt;
			pos += camera->GetRightVector() * moveInX * GetCameraSpeed(cameraSpeed) * dt;
			camera->SetPosition(pos);
		}
	}
	if (bMouseMiddle)
	{
		FVector pos = camera->GetPosition();
		pos += camera->GetUpVector() * mouseDeltaY * dt;
		pos += camera->GetRightVector() * -mouseDeltaX * dt;
		camera->SetPosition(pos);
	}

	mouseDeltaY = 0;
	mouseDeltaX = 0;
}

void CWorldViewportWidget::mouseMoveEvent(QMouseEvent *event)
{
	mouseDeltaX = ((float)event->x() - (float)prevMouseX) * 0.25f;
	mouseDeltaY = ((float)event->y() - (float)prevMouseY) * 0.25f;
	prevMouseX = event->x();
	prevMouseY = event->y();
}

void CWorldViewportWidget::mousePressEvent(QMouseEvent *event)
{
	if (bMouseLeft || bMouseMiddle || bMouseRight)
		return;

	switch (event->button())
	{
	case Qt::LeftButton:
		bMouseLeft = true;
		break;
	case Qt::RightButton:
	{
		QCursor cursor(Qt::BlankCursor);
		QApplication::setOverrideCursor(cursor);
		mouseClickPos = event->globalPos();
		bMouseRight = true;
	}
		break;
	case Qt::MiddleButton:
		bMouseMiddle = true;
		break;
	}
}

void CWorldViewportWidget::mouseReleaseEvent(QMouseEvent *event)
{
	switch (event->button())
	{
	case Qt::LeftButton:
		bMouseLeft = false;
		break;
	case Qt::RightButton:
		if (bMouseRight)
		{
			QApplication::restoreOverrideCursor();
			QCursor::setPos(mouseClickPos);
			bMouseRight = false;
		}
		break;
	case Qt::MiddleButton:
		bMouseMiddle = false;
		break;
	}
}

void CWorldViewportWidget::keyPressEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_W:
		moveInY += 1;
		break;
	case Qt::Key_S:
		moveInY -= 1;
		break;
	case Qt::Key_A:
		moveInX -= 1;
		break;
	case Qt::Key_D:
		moveInX += 1;
		break;
	}
}

void CWorldViewportWidget::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_W:
		moveInY -= 1;
		break;
	case Qt::Key_S:
		moveInY += 1;
		break;
	case Qt::Key_A:
		moveInX += 1;
		break;
	case Qt::Key_D:
		moveInX -= 1;
		break;
	}
}

void CWorldViewportWidget::wheelEvent(QWheelEvent *event)
{
	if (bMouseRight)
	{
		int d = event->delta() > 0 ? 1 : (event->delta() < 0 ? -1 : 0);
		cameraSpeed = FMath::Clamp(cameraSpeed + d, 1, 14);
	}
}
