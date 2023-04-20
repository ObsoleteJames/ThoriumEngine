
#include "InputManager.h"

#include "Game/PlayerController.h"
#include "Game/Pawn.h"

void CInputManager::SetInputWindow(IBaseWindow* window)
{
	if (inputWindow)
	{
		inputWindow->OnKeyEvent.RemoveAll(this);
		inputWindow->OnCharEvent.RemoveAll(this);
		inputWindow->OnCursorMove.RemoveAll(this);
		inputWindow->OnMouseButton.RemoveAll(this);
	}

	inputWindow = window;

	window->OnKeyEvent.Bind(this, &CInputManager::OnKeyEvent);
	window->OnCharEvent.Bind(this, &CInputManager::OnCharEvent);
	window->OnCursorMove.Bind(this, &CInputManager::OnCursorMove);
	window->OnMouseButton.Bind(this, &CInputManager::OnMouseButton);
}

void CInputManager::RegisterPlayer(CPlayerController* player)
{
	auto* pawn = player->GetPawn();
	pawn->SetupInput(this);
}

void CInputManager::SetInputMode(EInputMode mode)
{
	inputMode = mode;
}

void CInputManager::SetShowCursor(bool b)
{
	bShowCursor = b;
	if (inputWindow)
		inputWindow->SetCursorMode(bShowCursor ? ECursorMode::NORMAL : ECursorMode::DISABLED);
}

void CInputManager::OnKeyEvent(EKeyCode key, EInputAction action, EInputMod mod)
{

}

void CInputManager::OnCharEvent(uint key)
{

}

void CInputManager::OnCursorMove(double x, double y)
{

}

void CInputManager::OnMouseButton(EMouseButton btn, EInputAction action, EInputMod mod)
{

}
