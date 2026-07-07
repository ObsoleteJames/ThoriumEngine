
#include "InputManager.h"
#include "Engine.h"
#include "InputEvents.h"

#include "Game/GameInstance.h"
#include "Game/PlayerController.h"
#include "Game/Pawn.h"
#include "Game/UserInterface/Canvas.h"
#include "Console.h"
#include "InputDevice.h"

#include <set>
#include <Util/KeyValue.h>

void CInputManager::SetInputWindow(IBaseWindow* window)
{
	CONSOLE_LogWarning("CInputManager", "SetInputWindow is no longer used, use AddInputDevice instead!");
}

void CInputManager::AddInputDevice(IInputDevice* device)
{
	inputDevices.Add(device);

	if (bAutoAssignDevices)
	{
		// if there's only one player then that player gets input from all devices
		if (players.Size() == 1)
			device->assignedPlayer = players[0];
		else if (players.Size() > 1)
			device->assignedPlayer = *players.last();
	}

	if ((device->GetDeviceType() & Input_Keyboard) && !keyboardDevice)
		keyboardDevice = device;

	CONSOLE_LogInfo("CInputManager", "Input Device Added: '" + device->Name() + "' (Type: " + FString::ToString(device->GetDeviceType()) + ", ID: " + FString::ToString(device->GetDeviceId()) + ")");

	device->onInputEvent.Bind(this, [=](IInputEvent* event) { HandleInputEvent(device, event); });
	onInputDeviceAdded.Invoke(device);
}

void CInputManager::RemoveInputDevice(IInputDevice* device)
{
	auto it = inputDevices.Find(device);
	if (it != inputDevices.end())
		inputDevices.Erase(it);

	if (keyboardDevice == device)
		keyboardDevice = nullptr;

	CONSOLE_LogInfo("CInputManager", "Input Device Removed: '" + device->Name() + "' (Type: " + FString::ToString(device->GetDeviceType()) + ", ID: " + FString::ToString(device->GetDeviceId()) + ")");

	device->onInputEvent.RemoveAll(this);
}

void CInputManager::LoadConfig()
{
	FString cfgPath = gEngine->GetGameConfigPath() + "/input.cfg";
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
			if (k.type == 0) // Key
			{
				if (k.bNegate)
					a.cache += -(float)keyStates[k.key];
				else
					a.cache += keyStates[k.key];
			}
			else if (k.type == 1) // mouseKey
			{
				if (k.bNegate)
					a.cache += -(float)mouseStates[k.key];
				else
					a.cache += mouseStates[k.key];
			}
			else if (k.type == 2) // mouseAxis (position)
				a.cache += k.key == 1 ? mouseDelta.x : k.key == 2 ? mouseDelta.y : 0.f;
		}
	}

	if (!bEnableInput)
		return;

	if (inputMode != EInputMode::UI_ONLY)
	{
		for (auto& a : axis)
		{
			a.FireBindings(keyboardDevice);
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
	// Moved to CGameMode::SpawnPlayer
	//auto* pawn = player->GetPawn();
	//pawn->SetupInput(this);

	players.Add(player);

	if (bAutoAssignDevices)
	{
		if (players.Size() == 1)
		{
			for (auto& d : inputDevices)
				d->assignedPlayer = player;
		}
		else if (players.Size() > 1)
		{
			for (auto& d : inputDevices)
				if (d->assignedPlayer == nullptr)
					d->assignedPlayer = player;
		}
	}
}

void CInputManager::RemovePlayer(CPlayerController* player)
{
	CPawn* pawn = player->GetPawn();
	if (pawn)
		UnbindPawn(pawn);

	// clear device assignments for this player
	for (auto& d : inputDevices)
		if (d->assignedPlayer == player)
			d->assignedPlayer = nullptr;

	auto it = players.Find(player);
	if (it != players.end())
		players.Erase(it);
}

void CInputManager::AssignPlayerDevice(CPlayerController* target, IInputDevice* device)
{
	if (device->assignedPlayer != nullptr)
		CONSOLE_LogInfo("CInputManager", "Input Device '" + device->Name() + "' has been re-assigned");

	device->assignedPlayer = target;
}

void CInputManager::UnassignPlayerDevice(IInputDevice* device)
{
	device->assignedPlayer = nullptr;
}

void CInputManager::UnassignAllDevices()
{
	for (auto& d : inputDevices)
		d->assignedPlayer = nullptr;
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
	//if (inputWindow)
	//	inputWindow->SetCursorMode(bShowCursor ? ECursorMode::NORMAL : ECursorMode::DISABLED);
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

void CInputManager::CopyState(CInputManager* other)
{
	//SetInputWindow(other->inputWindow);

	bEnableInput = other->bEnableInput;
	bShowCursor = other->bShowCursor;
	bAutoAssignDevices = other->bAutoAssignDevices;
	inputMode = other->inputMode;

	keyBindings = other->keyBindings;
	actions = other->actions;
	axis = other->axis;

	players = other->players;
	inputDevices = other->inputDevices;
	keyboardDevice = other->keyboardDevice;

	SetInputMode(inputMode);

	CONSOLE_LogInfo("CInputManager", "Input manager copied state");
}

void CInputManager::UnbindPawn(CPawn* pawn)
{
	for (auto& action : actions)
	{
		for (auto it = action.bindings.rbegin(); it != action.bindings.rend(); it++)
		{
			if (it->pawn == pawn)
				action.bindings.Erase(it);
		}
	}
	for (auto& a : axis)
	{
		for (auto it = a.bindings.rbegin(); it != a.bindings.rend(); it++)
		{
			if (it->pawn == pawn)
				a.bindings.Erase(it);
		}
	}
}

void CInputManager::CharEvent(IInputDevice* device, CKeyEvent* event)
{
	// TODO: invoke event to UI.
}

void CInputManager::KeyEvent(IInputDevice* device, CKeyEvent* event)
{
	if (!bEnableInput)
		return;

	keyStates[(SizeType)event->Key()] = (event->Action() != IE_RELEASE);

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

	for (auto& a : keyBindings)
	{
		if (a.key == (uint16)event->Key() && a.mods == event->Mods() && a.activactionAction == event->Action())
		{
			if (a.pawn != device->assignedPlayer->GetPawn())
				continue;

			if (a.layer != inputMode && a.layer != EInputMode::GAME_UI)
				continue;

			a.binding.Invoke();
		}
	}

	for (auto& a : actions)
	{
		for (auto& k : a.keys)
		{
			if (k.type == 0 && k.key == (uint16)event->Key() && k.mods == event->Mods())
			{
				a.FireBindings(device, event->Action());
			}
		}
	}
}

void CInputManager::CursorMove(IInputDevice* device, CMouseEvent* event)
{
	mousePos = event->GetMousePos();
}

void CInputManager::MouseButton(IInputDevice* device, CMouseEvent* event)
{
	if (!bEnableInput)
		return;

	mouseStates[(SizeType)event->Button()] = (event->Action() != IE_RELEASE);

	for (auto& a : actions)
	{
		for (auto& k : a.keys)
		{
			if (k.type == 1 && k.key == (uint16)event->Button() && k.mods == event->Mods())
			{
				a.FireBindings(device, event->Action());
			}
		}
	}
}

void CInputManager::HandleInputEvent(IInputDevice* device, IInputEvent* event)
{
	onInputEvent.Invoke(device, event);

	// ignore the event if it's already been handled by onInputEvent.
	if (event->Accepted())
		return;

	switch (event->Type())
	{
	case IInputEvent::KeyEvent:
		KeyEvent(device, (CKeyEvent*)event);
		break;

	case IInputEvent::CharEvent:
		CharEvent(device, (CKeyEvent*)event);
		break;

	case IInputEvent::MouseButton:
		MouseButton(device, (CMouseEvent*)event);
		break;

	case IInputEvent::MouseMove:
		CursorMove(device, (CMouseEvent*)event);
		break;
	}
}

void CInputManager::OnDelete()
{
	for (auto& d : inputDevices)
		d->onInputEvent.RemoveAll(this);

	inputDevices.Clear();

	BaseClass::OnDelete();
}

void FInputAction::FireBindings(IInputDevice* device, EInputAction action)
{
	for (auto& b : bindings)
	{
		if (b.pawn != device->GetAssignedPlayer()->GetPawn())
			continue;

		if (b.layer != gEngine->InputManager()->GetInputMode() && b.layer != EInputMode::GAME_UI)
			continue;

		if (b.activactionAction == action)
			b.binding.Invoke();
	}
}

void FInputAxis::FireBindings(IInputDevice* device)
{
	for (auto& b : bindings)
	{
		if (b.pawn != device->GetAssignedPlayer()->GetPawn())
			continue;

		if (b.layer != gEngine->InputManager()->GetInputMode() && b.layer != EInputMode::GAME_UI)
			continue;

		b.binding.Invoke(cache);
	}
}
