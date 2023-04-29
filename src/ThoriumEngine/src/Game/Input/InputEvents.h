#pragma once

#include "Window.h"
#include "Math/Vectors.h"

class CKeyEvent
{
public:
	CKeyEvent(EKeyCode k, EInputAction a, EInputMod m) : key(k), action(a), mods(m), bConsumed(false) {}

	void Accept() { bConsumed = true; }
	inline bool Accepted() const { return bConsumed; }

	inline EKeyCode Key() const { return key; }
	inline EInputAction Action() const { return action; }
	inline EInputMod Mods() const { return mods; }

private:
	bool bConsumed;
	EKeyCode key;
	EInputAction action;
	EInputMod mods;
};

class CMouseEvent
{
public:
	CMouseEvent(EMouseButton b, EInputAction a, EInputMod m, FVector2 mp) : btn(b), action(a), mods(m), mousePos(mp), bConsumed(false) {}

	void Accept() { bConsumed = true; }
	inline bool Accepted() const { return bConsumed; }

	inline EMouseButton Button() const { return btn; }
	inline EInputAction Action() const { return action; }
	inline EInputMod Mods() const { return mods; }

	inline const FVector2& GetMousePos() const { return mousePos; }

	inline bool IsMoveEvent() { return btn == EMouseButton::NONE; }

private:
	bool bConsumed;
	EMouseButton btn;
	EInputAction action;
	EInputMod mods;

	FVector2 mousePos;
};
