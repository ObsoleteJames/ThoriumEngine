#pragma once

#include "Window.h"
#include "Math/Vectors.h"

class ENGINE_API IInputEvent
{
public:
	enum EType
	{
		KeyEvent = 1,
		CharEvent, // character events, used for input fields
		MouseButton,
		MouseMove,
		MouseScroll,
		GamepadButton,
		GamepadAxis,
	};

public:
	IInputEvent() = default;

	inline void Accept() { bConsumed = true; }
	inline bool Accepted() const { return bConsumed; }

	inline uint8 Type() const { return type; }

protected:
	bool bConsumed = false;
	uint8 type = 0;
	uint32 deviceId;
};

class ENGINE_API CKeyEvent : public IInputEvent
{
public:
	CKeyEvent(EKeyCode k, EInputAction a, EInputMod m, IInputEvent::EType type) : key(k), action(a), mods(m) { this->type = type; }

	inline EKeyCode Key() const { return key; }
	inline EInputAction Action() const { return action; }
	inline EInputMod Mods() const { return mods; }

private:
	EKeyCode key;
	EInputAction action;
	EInputMod mods;
};

class ENGINE_API CMouseEvent : public IInputEvent
{
public:
	CMouseEvent(EMouseButton b, EInputAction a, EInputMod m, FVector2 mp, IInputEvent::EType type) : btn(b), action(a), mods(m), mousePos(mp) { this->type = type; }

	inline EMouseButton Button() const { return btn; }
	inline EInputAction Action() const { return action; }
	inline EInputMod Mods() const { return mods; }

	inline const FVector2& GetMousePos() const { return mousePos; }

private:
	EMouseButton btn;
	EInputAction action;
	EInputMod mods;

	FVector2 mousePos;
};

class ENGINE_API CAxisEvent : public IInputEvent
{
public:
	CAxisEvent(uint16 key, float x, float y, IInputEvent::EType type) : key(key), x(x), y(y) { this->type = type; }

	inline float GetX() const { return x; }
	inline float GetY() const { return y; }

private:
	uint16 key;
	float x, y;
};
