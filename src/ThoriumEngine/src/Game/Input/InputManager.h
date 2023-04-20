#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "Window.h"
#include "InputManager.generated.h"

class CPlayerController;

ENUM()
enum class EInputMode
{
	GAME_ONLY,
	UI_ONLY,
	GAME_UI
};

CLASS()
class ENGINE_API CInputManager : public CObject
{
	GENERATED_BODY()

public:
	virtual void SetInputWindow(IBaseWindow* window);

	virtual void BuildInput() {}
	virtual void ClearCache() {}

	virtual void RegisterPlayer(CPlayerController* player);

	virtual void SetInputMode(EInputMode mode);
	inline EInputMode GetInputMode() const { return inputMode; }

	void SetShowCursor(bool b);
	inline bool CursorVisible() const { return bShowCursor; }

protected:
	void OnKeyEvent(EKeyCode key, EInputAction action, EInputMod mod);
	void OnCharEvent(uint key);

	void OnCursorMove(double x, double y);
	void OnMouseButton(EMouseButton btn, EInputAction action, EInputMod mod);

protected:
	bool bShowCursor;
	EInputMode inputMode;
	IBaseWindow* inputWindow;
};
