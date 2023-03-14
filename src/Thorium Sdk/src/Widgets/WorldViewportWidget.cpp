
#include <string>
#include "EditorEngine.h"
#include "WorldViewportWidget.h"
#include "Game/Components/CameraComponent.h"
#include "Rendering/RenderScene.h"
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

	moveForward = 0;
	moveBack = 0;
	moveLeft = 0;
	moveRight = 0;
}

CWorldViewportWidget::~CWorldViewportWidget()
{
}

void CWorldViewportWidget::Update(double dt)
{
	if (bMouseRight && mode == ECameraControlMode::FreeMode)
	{
		cameraPitch = FMath::Clamp(cameraPitch + mouseDeltaY, -90.f, 90.f);
		cameraYaw += mouseDeltaX;
		camera->SetRotation(FQuaternion::EulerAngles(FVector(cameraPitch, cameraYaw, 0.f).Radians()));

		FVector move = GetMoveVector();

		if (move.Magnitude() != 0.0f)
		{
			if (curSpeed < GetCameraSpeed(cameraSpeed))
				curSpeed += (5.f * (float)cameraSpeed) * (float)dt;
			else
				curSpeed = GetCameraSpeed(cameraSpeed);

			FVector pos = camera->GetPosition();
			pos += camera->GetForwardVector() * move.y * curSpeed * dt;
			pos += camera->GetRightVector() * move.x * curSpeed * dt;
			camera->SetPosition(pos);
		}
		else
			curSpeed = 0.f;
	}
	if (bMouseLeft && mode == ECameraControlMode::Orbit)
	{
		cameraPitch = FMath::Clamp(cameraPitch + mouseDeltaY, -90.f, 90.f);
		cameraYaw += mouseDeltaX;
		camera->SetRotation(FQuaternion::EulerAngles(FVector(cameraPitch, cameraYaw, 0.f).Radians()));
	}
	if (bMouseMiddle)
	{
		if (mode == ECameraControlMode::FreeMode)
		{
			FVector pos = camera->GetPosition();
			pos += camera->GetUpVector() * mouseDeltaY * dt;
			pos += camera->GetRightVector() * -mouseDeltaX * dt;
			camera->SetPosition(pos);
		}
		else
		{
			orbitPos += camera->GetUpVector() * mouseDeltaY * dt;
			orbitPos += camera->GetRightVector() * -mouseDeltaX * dt;
		}
	}

	if (mode == ECameraControlMode::Orbit)
		camera->SetPosition((-camera->GetForwardVector() * (15 - cameraSpeed)) + orbitPos);

	mouseDeltaY = 0;
	mouseDeltaX = 0;
}

void CWorldViewportWidget::mouseMoveEvent(QMouseEvent *event)
{
	mouseDeltaX = ((float)event->x() - (float)prevMouseX) * 0.25f;
	mouseDeltaY = ((float)event->y() - (float)prevMouseY) * 0.25f;
	prevMouseX = event->x();
	prevMouseY = event->y();
	CRenderWidget::mouseMoveEvent(event);
}

void CWorldViewportWidget::mousePressEvent(QMouseEvent *event)
{
	CRenderWidget::mousePressEvent(event);
	if (bMouseLeft || bMouseMiddle || bMouseRight)
		return;

	switch (event->button())
	{
	case Qt::LeftButton:
		bMouseLeft = true;
		break;
	case Qt::RightButton:
	{
		if (mode == ECameraControlMode::FreeMode)
		{
			QCursor cursor(Qt::BlankCursor);
			QApplication::setOverrideCursor(cursor);
			mouseClickPos = event->globalPos();
		}
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
	CRenderWidget::mouseReleaseEvent(event);
	switch (event->button())
	{
	case Qt::LeftButton:
		bMouseLeft = false;
		break;
	case Qt::RightButton:
		if (bMouseRight)
		{
			if (mode == ECameraControlMode::FreeMode)
			{
				QApplication::restoreOverrideCursor();
				QCursor::setPos(mouseClickPos);
			}
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
		moveForward = 1;
		break;
	case Qt::Key_S:
		moveBack = 1;
		break;
	case Qt::Key_A:
		moveLeft = 1;
		break;
	case Qt::Key_D:
		moveRight = 1;
		break;
	}
	CRenderWidget::keyPressEvent(event);
}

void CWorldViewportWidget::keyReleaseEvent(QKeyEvent *event)
{
	switch (event->key())
	{
	case Qt::Key_W:
		moveForward = 0;
		break;
	case Qt::Key_S:
		moveBack = 0;
		break;
	case Qt::Key_A:
		moveLeft = 0;
		break;
	case Qt::Key_D:
		moveRight = 0;
		break;
	}
	CRenderWidget::keyReleaseEvent(event);
}

void CWorldViewportWidget::wheelEvent(QWheelEvent *event)
{
	if (bMouseRight || mode== ECameraControlMode::Orbit)
	{
		int d = event->delta() > 0 ? 1 : (event->delta() < 0 ? -1 : 0);
		cameraSpeed = FMath::Clamp(cameraSpeed + d, 1, 14);
	}
}

void CWorldViewportWidget::SetCamera(CCameraComponent* cam)
{
	camera = cam;
	FVector euler = camera->GetWorldRotation().ToEuler();
	euler = FMath::Degrees(euler);
	cameraPitch = euler.x;
	cameraYaw = euler.y;

	scene->SetCamera(camera);
}
