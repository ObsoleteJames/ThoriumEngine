#pragma once

#include <QWidget>

class CCamera;

class CEditorViewport : public QWidget
{
	Q_OBJECT

public:

private:
	bool bCanRotate;
	CCamera* camera;

};
