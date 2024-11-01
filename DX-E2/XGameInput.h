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
	RIGHT_BUMPER,

	PLACE_HOLDER_1,
	PLACE_HOLDER_2,

	A_BUTTON,
	B_BUTTON,
	X_BUTTON,
	Y_BUTTON
};

enum class VariableActionMappingEnum : uint8_t
{
	MOVE_FORWARDS,
	MOVE_BACKWARDS,
	MOVE_RIGHT,
	MOVE_LEFT,

	MAX
};

enum class VariableInputStrengthsEnum : uint8_t
{
	MOVE_ON_FORWARD_VECTOR,
	MOVE_ON_RIGHT_VECTOR,

	MAX
};

//From here onward these have 1:1 mapping with Controller, Keyboard, Mouse buttons.
enum class DirectButtonActionsEnum : uint8_t
{
	//Toggle/Hold Actions
	SPRINT,
	CROUCHING,
	HOLD_LOOK,

	//Non Toggle/Hold Actions.
	JUMPING,
	PRIMARY_FIRE,
	SECONDARY_FIRE,
	SWAP_TO_NEXT_WEAPON,
	SWAP_TO_PREVIOUS_WEAPON,
	SWAP_TO_MELEE_WEAPON,
	DIVE_TO_PRONE,
	LAY_TO_PRONE,

	SELECTION_CONFIRM_BUTTON,  // Spacebar, Enter, XBOX_A, etc.
	BACK_OR_CANCEL_BUTTON,  // XBOX_B, ESC, Backspace.
	CHANGE_TAB_PREVIOUS,  // SHIFT LEFT, LEFT BUMPER, LEFT TRIGGER.
	CHANGE_TAB_NEXT,  // SHIFT RIGHT, TAB, RIGHT BUMPER, RIGHT TRIGGER.
	CHANGE_SELECTION_PREVIOUS,  // A, ARROW_LEFT, DPAD_LEFT, Joystick's Left.
	CHANGE_SELECTION_NEXT,   // D, ARROW_RIGHT, DPAD_RIGHT, Joystick's Right.

	//End Of Actions
	MAX
};

class XGameInput
{
	public:

		static void InitializeDefaultConfigurations();
		static bool LoadAndProcessXboxInputChanges();

		static void GameInputPostProcessing();
		static void StoreRawInputStateChanges(RAWINPUT* &RawInput);

		static int16_t GetRightMovementStrength();
		static int16_t GetForwardMovementStrength();
		static int16_t GetRightStickX();
		static int16_t GetRightStickY();

		//Toggle/Hold style'd Actions are accessed here.
		static bool ActionWasInitiated(DirectButtonActionsEnum Action);
		static bool ActionIsCurrentlyActive(DirectButtonActionsEnum Action);
		static bool ActionHasEnded(DirectButtonActionsEnum Action);

		static bool HasFlagSettings(const uint8_t FlagSettings);

	public:

		static uint64_t MouseCalls;
};
