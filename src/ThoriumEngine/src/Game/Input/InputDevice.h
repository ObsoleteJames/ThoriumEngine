#pragma once

#include "Object/Object.h"
#include "InputEvents.h"
#include "InputDevice.generated.h"

class CPlayerController;

ENUM()
enum EInputDeviceType
{
	Input_Keyboard = 1,
	Input_Mouse = 1 << 1,
	Input_Gamepad = 1 << 2,
	Input_Other = 1 << 3
};

CLASS(Abstract)
class ENGINE_API IInputDevice : public CObject
{
	GENERATED_BODY()

	friend class CInputManager;

public:
	IInputDevice() = default;

	inline uint8 GetDeviceType() const { return deviceType; }
	inline uint32 GetDeviceId() const { return deviceId; }

	inline CPlayerController* GetAssignedPlayer() const { return assignedPlayer; }

public:
	TDelegate<IInputEvent*> onInputEvent;

protected:
	uint8 deviceType;
	uint32 deviceId;

private:
	// the player that this device is assigned to, this is controlled by the input manager.
	CPlayerController* assignedPlayer = nullptr;
};
