
#include "InputManager.h"
#include "Engine.h"
#include "InputEvents.h"

#include "Game/GameInstance.h"
#include "Game/PlayerController.h"
#include "Game/Pawn.h"
#include "Game/UserInterface/Canvas.h"

#include <set>
#include <Util/KeyValue.h>

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

	// TODO: Fix crashing when in editor.
	if (!gIsEditor)
		window->OnKeyEvent.Bind(this, &CInputManager::KeyEvent);

	window->OnCharEvent.Bind(this, &CInputManager::OnCharEvent);
	window->OnCursorMove.Bind(this, &CInputManager::OnCursorMove);
	window->OnMouseButton.Bind(this, &CInputManager::OnMouseButton);
}

void CInputManager::LoadConfig()
{
	WString cfgPath = gEngine->GetGameConfigPath() + L"\\input.cfg";
	FKeyValue kv(cfgPath);
	if (!kv.IsOpen())
		return;

	if (KVCategory* cActions = kv.GetCategory("Actions"); cActions)
	{
		for (auto* inputs : cActions->GetCategories())
		{
			FInputAction input;
			input.name = inputs->GetName();
			
			for (auto* keys : inputs->GetCategories())
			{
				FInputActionKey key;
				key.type = keys->GetValue("Type")->AsInt();
				key.mods = (EInputMod)keys->GetValue("Mods")->AsInt();

				switch (key.type)
				{
				case 0:
				{
					FEnum* keyEnum = CModuleManager::FindEnum("EKeyCode");
					if (!keyEnum)
						break;

					key.key = keyEnum->GetValueByName(*keys->GetValue("Key"));
				}
					break;
				case 1:
				{
					FEnum* mouseEnum = CModuleManager::FindEnum("EMouseButton");
					if (!mouseEnum)
						break;

					key.key = mouseEnum->GetValueByName(*keys->GetValue("Key"));
				}
					break;
				}

				input.keys.Add(key);
			}

			actions.Add(input);
		}
	}
}

void CInputManager::SaveConfig()
{

}

void CInputManager::BuildInput()
{

}

void CInputManager::ClearCache()
{

}

void CInputManager::RegisterPlayer(CPlayerController* player)
{
	auto* pawn = player->GetPawn();
	pawn->SetupInput(this);
}

void CInputManager::RemovePlayer(CPlayerController* player)
{
	for (auto& action : actions)
	{
		for (auto it = action.bindings.rbegin(); it != action.bindings.rend(); it++)
		{
			if (it->player == player)
				action.bindings.Erase(it);
		}
	}
	for (auto& a : axis)
	{
		for (auto it = a.bindings.rbegin(); it != a.bindings.rend(); it++)
		{
			if (it->player == player)
				a.bindings.Erase(it);
		}
	}
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

FInputAction* CInputManager::GetAction(const FString& name)
{
	for (auto& a : actions)
		if (a.name == name)
			return &a;

	return nullptr;
}

FInputAxis* CInputManager::GetAxis(const FString& name)
{
	for (auto& a : axis)
		if (a.name == name)
			return &a;

	return nullptr;
}

void CInputManager::KeyEvent(EKeyCode key, EInputAction action, EInputMod mod)
{
	CKeyEvent event(key, action, mod);

	if (inputMode != EInputMode::GAME_ONLY)
	{
		//std::set<int, CCanvas*> canvass;
		//for (auto c : gEngine->GameInstance()->GetGlobalCanvass())
		//	canvass.emplace(c->ZOrder(), c);

		//for (auto p : gEngine->GameInstance()->GetPlayers())
		//{
		//	for (auto c : p->GetPlayerController()->GetCanvass())
		//		canvass.emplace(-c->ZOrder(), c);
		//}

		//std::sort(canvass.begin(), canvass.end());

		/*for (auto it = canvass.begin(); it != canvass.end(); it++)
		{


			if (event.Accepted())
				break;
		}*/
	}
	
	if (inputMode != EInputMode::UI_ONLY)
	{
		for (auto& a : actions)
		{
			for (auto& k : a.keys)
			{
				if (k.type == 0 && k.key == (uint16)key && k.mods == mod)
				{
					a.FireBindings(action);
				}
			}
		}
	}

	// This won't work
	//for (auto& a : axis)
	//{
	//	for (auto& k : a.keys)
	//	{
	//		if (k.type == 0 && k.key == (uint16)key && action == IE_PRESS)
	//		{
	//			a.cache += k.bNegate ? -1.f : 1.f;
	//		}
	//	}
	//}
}

void CInputManager::OnCharEvent(uint key)
{

}

void CInputManager::OnCursorMove(double x, double y)
{

}

void CInputManager::OnMouseButton(EMouseButton btn, EInputAction action, EInputMod mod)
{
	for (auto& a : actions)
	{
		for (auto& k : a.keys)
		{
			if (k.type == 1 && k.key == (uint16)btn && k.mods == mod)
			{
				a.FireBindings(action);
			}
		}
	}
}

void FInputAction::FireBindings(EInputAction action)
{
	for (auto& b : bindings)
	{
		if (b.activactionAction == action)
			b.binding.Invoke();
	}
}

void FInputAxis::FireBindings()
{
	for (auto& b : bindings)
	{
		b.binding.Invoke(cache);
	}
}
