#pragma once

#include <Windows.h>
#include <Xinput.h>

#include <cstdint>

enum XboxControllerButtonIndexes : uint8_t
{
	D_PAD_UP,
	D_PAD_DOWN,
	D_PAD_LEFT,
	D_PAD_RIGHT,

	MENU_BUTTON,
	VIEW_BUTTON,

	LEFT_STICK_CLICK,
	RIGHT_CLICK_STICK,

	LEFT_BUMPER,
	RIGHT_NUMPER,

	PLACE_HOLDER_1,
	PLACE_HOLDER_2,

	A_BUTTON,
	B_BUTTON,
	X_BUTTON,
	Y_BUTTON
};

enum class InputActions : uint8_t
{
	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_RIGHT,
	MOVE_LEFT,
	SPRINT,
	HOLD_LOOK,
	CROUCHING,
	JUMPING,

	MAX
};

class XGameInput
{
public:

	static void InitializeDefaultConfigurations();
	static bool LoadController();

	static void GameInputPostProcessing();
	static void StoreRawInputStateChanges(RAWINPUT* &RawInput);

	static uint64_t MouseCalls;

	static uint16_t GetControllerButtonsPressed(uint16_t ButtonValues);

	static int16_t GetLeftStickX();
	static int16_t GetLeftStickY();
	static int16_t GetRightStickX();
	static int16_t GetRightStickY();

	static bool IsControllerActionHeld(InputActions Action);
	static bool IsControllerActionPressed(InputActions Action);
	static bool IsControllerActionReleased(InputActions Action);

	static bool IsPCInputActionHeld(InputActions Action);
	static bool IsPCInputActionPressed(InputActions Action);
	static bool IsPCInputActionReleased(InputActions Action);
};
