#pragma once

#include <Object/Object.h>

class CLayer : public CObject
{
public:
	virtual ~CLayer() = default;

	virtual void OnAttach() {}
	virtual void OnDetach() {}

	virtual void OnUpdate(double dt) {}
	virtual void OnUIRender() {}

public:
	bool bEnabled = true;
};
