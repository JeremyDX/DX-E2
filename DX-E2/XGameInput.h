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
	//This specific Actions although tied to WASD for instance are not regularly used they are compared against controlled Joysticks.
	MOVE_FORWARD,
	MOVE_BACKWARD,
	MOVE_RIGHT,
	MOVE_LEFT,

	//From here onward these have 1:1 mapping with Controller, Keyboard, Mouse buttons.
	SPRINT,
	HOLD_LOOK,
	CROUCHING,
	JUMPING,

	//Actions that hold their Press/Release actions.
	INTERFACE_ACCEPT,
	INTERFACE_BACK,

	//End Of Actions
	MAX
};

class XGameInput
{
	public:

		static void InitializeDefaultConfigurations();
		static bool LoadController();

		static void GameInputPostProcessing();
		static void StoreRawInputStateChanges(RAWINPUT* &RawInput);

		static int16_t GetLeftStickX();
		static int16_t GetLeftStickY();
		static int16_t GetRightStickX();
		static int16_t GetRightStickY();

		static bool ActionHasStarted(InputActions Action);
		static bool ActionIsCurrentlyActive(InputActions Action);
		static bool ActionHasEnded(InputActions Action);

	public:

		static uint64_t MouseCalls;
};
