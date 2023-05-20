
#include "InputManager.h"
#include "Engine.h"
#include "InputEvents.h"

#include "Game/GameInstance.h"
#include "Game/PlayerController.h"
#include "Game/Pawn.h"
#include "Game/UserInterface/Canvas.h"
#include "Console.h"

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

	SetInputMode(inputMode);
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
				FInputActionKey key{};
				key.type = keys->GetValue("Type")->AsInt();
				key.mods = (EInputMod)keys->GetValue("Mods")->AsInt();

				switch (key.type)
				{
				case 0:
				{
					FEnum* keyEnum = CModuleManager::FindEnum("EKeyCode");
					if (!keyEnum)
						break;

					FString keyValue = *keys->GetValue("Key");
					key.key = (uint16)keyEnum->GetValueByName(keyValue);
				}
					break;
				case 1:
				{
					FEnum* mouseEnum = CModuleManager::FindEnum("EMouseButton");
					if (!mouseEnum)
						break;

					key.key = (uint16)mouseEnum->GetValueByName(*keys->GetValue("Key"));
				}
					break;
				}

				input.keys.Add(key);
			}

			actions.Add(input);
		}
	}

	if (KVCategory* cAxis = kv.GetCategory("Axis"); cAxis)
	{
		for (auto* inputs : cAxis->GetCategories())
		{
			FInputAxis input;
			input.name = inputs->GetName();

			for (auto* keys : inputs->GetCategories())
			{
				FInputAxisKey key{};
				key.type = keys->GetValue("Type")->AsInt();
				key.bNegate = keys->GetValue("Negate")->AsBool();

				switch (key.type)
				{
				case 0:
				{
					FEnum* keyEnum = CModuleManager::FindEnum("EKeyCode");
					if (!keyEnum)
						break;

					FString keyValue = *keys->GetValue("Key");
					key.key = (uint16)keyEnum->GetValueByName(keyValue);
				}
					break;
				case 1:
				{
					FEnum* mouseEnum = CModuleManager::FindEnum("EMouseButton");
					if (!mouseEnum)
						break;

					key.key = (uint16)mouseEnum->GetValueByName(*keys->GetValue("Key"));
				}
					break;
				case 2:
				{
					FString v = *keys->GetValue("Key");
					key.key = v == "MOUSE_X" ? 1 : v == "MOUSE_Y" ? 2 : 0;
				}
					break;
				}

				input.keys.Add(key);
			}

			axis.Add(input);
		}
	}
}

void CInputManager::SaveConfig()
{

}

void CInputManager::BuildInput()
{
	mouseDelta = mousePos - prevMousePos;
	prevMousePos = mousePos;

	for (auto& a : axis)
	{
		for (auto& k : a.keys)
		{
			if (k.type == 0)
			{
				if (k.bNegate)
					a.cache += -(float)keyStates[k.key];
				else
					a.cache += keyStates[k.key];
			}
			else if (k.type == 1)
			{
				if (k.bNegate)
					a.cache += -(float)mouseStates[k.key];
				else
					a.cache += mouseStates[k.key];
			}
			else if (k.type == 2)
				a.cache += k.key == 1 ? mouseDelta.x : k.key == 2 ? mouseDelta.y : 0.f;
		}
	}

	if (inputMode != EInputMode::UI_ONLY)
	{
		for (auto& a : axis)
		{
			a.FireBindings();
		}
	}
}

void CInputManager::ClearCache()
{
	for (auto& a : axis)
		a.cache = 0.f;
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

	if (inputMode == EInputMode::GAME_ONLY)
		SetShowCursor(false);
	else
		SetShowCursor(true);
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

	keyStates[(SizeType)key] = action != IE_RELEASE;

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
}

void CInputManager::OnCharEvent(uint key)
{

}

void CInputManager::OnCursorMove(double x, double y)
{
	mousePos = FVector2((float)x, (float)y);
}

void CInputManager::OnMouseButton(EMouseButton btn, EInputAction action, EInputMod mod)
{
	mouseStates[(SizeType)btn] = action != IE_RELEASE;

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
