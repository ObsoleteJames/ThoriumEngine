#pragma once

#include "EngineCore.h"
#include "Object/Object.h"
#include "Game/Pawn.h"
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

struct FInputActionBinding
{
	FString actionName;
	EInputAction activactionAction;

	TObjectPtr<CPlayerController> player;
	TDelegate<> binding;
	EInputMode layer;
};

struct FInputAxisBinding
{
	FString axisName;

	TObjectPtr<CPlayerController> player;
	TDelegate<float> binding;
	EInputMode layer;
};

struct FInputKeyBinding
{
	uint16 key;
	EInputMod mods;
	EInputAction activactionAction;

	TObjectPtr<CPlayerController> player;
	TDelegate<> binding;
	EInputMode layer;
};

struct FInputActionKey
{
	uint16 key;
	uint8 type; // 0 = key, 1 = mouse, 2 = gamepad
	EInputMod mods;
};

struct ENGINE_API FInputAction
{
public:
	FString name;
	TArray<FInputActionKey> keys;
	TArray<FInputActionBinding> bindings;

public:
	void FireBindings(EInputAction action);
};

struct FInputAxisKey
{
	uint16 key;
	uint8 type; // 0 = key, 1 = mouse, 2 = mouseAxis, 3 = gamepad, 4 = gamepadAxis
	bool bNegate;
};

struct ENGINE_API FInputAxis
{
public:
	FString name;
	TArray<FInputAxisKey> keys;
	TArray<FInputAxisBinding> bindings;
	float cache;

public:
	void FireBindings();
};

CLASS()
class ENGINE_API CInputManager : public CObject
{
	GENERATED_BODY()

public:
	virtual void SetInputWindow(IBaseWindow* window);

	virtual void LoadConfig();
	virtual void SaveConfig();

	virtual void BuildInput();
	virtual void ClearCache();

	virtual void RegisterPlayer(CPlayerController* player);
	virtual void RemovePlayer(CPlayerController* player);

	virtual void SetInputMode(EInputMode mode);
	inline EInputMode GetInputMode() const { return inputMode; }

	void SetShowCursor(bool b);
	inline bool CursorVisible() const { return bShowCursor; }
	
	inline bool InputEnabled() const { return bEnableInput; }
	inline void SetInputEnabled(bool b) { bEnableInput = b; }
	inline void EnableInput() { bEnableInput = true; }
	inline void DisableInput() { bEnableInput = false; }

	inline FVector2 GetMousePos() const { return mousePos; }

	inline TArray<FInputAction>& GetActions() { return actions; }
	inline TArray<FInputAxis>& GetAxis() { return axis; }

	FInputAction* GetAction(const FString& name);
	FInputAxis* GetAxis(const FString& name);

	void CopyState(CInputManager* other);

	template<typename T>
	void BindAction(FString name, EInputAction action, T* target, void(T::* func)(), EInputMode layer = EInputMode::GAME_ONLY);

	template<typename T>
	void BindAxis(FString name, T* target, void(T::* func)(float), EInputMode layer = EInputMode::GAME_ONLY);

	template<typename T>
	void BindKey(EKeyCode key, EInputAction action, EInputMod mods, T* target, void(T::* func)(), EInputMode layer = EInputMode::GAME_ONLY);

	inline bool IsKeyPressed(EKeyCode key) const { return (bool)keyStates[(int)key]; }
	inline bool IsMousePressed(EMouseButton btn) const { return (bool)mouseStates[(int)btn]; }

protected:
	void OnCharEvent(uint key);
	void KeyEvent(EKeyCode key, EInputAction action, EInputMod mod);

	void OnCursorMove(double x, double y);
	void OnMouseButton(EMouseButton btn, EInputAction action, EInputMod mod);

	void OnDelete() override;

protected:
	bool bEnableInput = true;
	bool bShowCursor = true;
	EInputMode inputMode;
	IBaseWindow* inputWindow = nullptr;

	TArray<FInputKeyBinding> keyBindings;
	TArray<FInputAction> actions;
	TArray<FInputAxis> axis;

	FVector2 mouseDelta;
	FVector2 mousePos;
	FVector2 prevMousePos;

	int8 keyStates[(SizeType)EKeyCode::KEYS_COUNT];
	int8 mouseStates[(SizeType)EMouseButton::NONE];
};

template<typename T>
void CInputManager::BindAction(FString name, EInputAction aAction, T* target, void(T::* func)(), EInputMode layer)
{
	FInputAction* action = GetAction(name);
	if (!action)
		return;

	action->bindings.Add();
	FInputActionBinding& ab = *action->bindings.last();
	ab.actionName = name;
	ab.activactionAction = aAction;
	ab.player = Cast<CPlayerController>(target->GetController());
	ab.binding.Bind(target, func);
	ab.layer = layer;
}

template<typename T>
void CInputManager::BindAxis(FString name, T* target, void(T::* func)(float), EInputMode layer)
{
	FInputAxis* axis = GetAxis(name);
	if (!axis)
		return;

	axis->bindings.Add();
	FInputAxisBinding& ab = *axis->bindings.last();
	ab.axisName = name;
	ab.player = Cast<CPlayerController>(target->GetController());
	ab.binding.Bind(target, func);
	ab.layer = layer;
}

template<typename T>
void CInputManager::BindKey(EKeyCode key, EInputAction action, EInputMod mods, T* target, void(T::* func)(), EInputMode layer)
{
	keyBindings.Add();
	FInputKeyBinding& ab = *keyBindings.last();
	ab.key = (uint16)key;
	ab.activactionAction = action;
	ab.mods = mods;
	ab.player = Cast<CPlayerController>(target->GetController());
	ab.binding.Bind(target, func);
	ab.layer = layer;
}
