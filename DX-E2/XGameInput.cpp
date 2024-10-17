#include "XGameInput.h"

#include "GameTime.h"
#include "Engine.h"
#include <cstdint>
#include <stdio.h>

constexpr const char* INPUT_ACTION_NAMES[static_cast<int>(InputActions::MAX)] =
{
	"Move Forward",  // 0
	"Move Backward", // 1
	"Move Right",    // 2
	"Move Left",     // 3
	"Sprint",        // 4
	"Hold Look",     // 5
	"Crouch",        // 6
	"Jumping",       // 7
};

XINPUT_STATE CurrentGamepadState = { };

//Storage for if a Mouse or Keyboard Button is Pressed or Released Currently on Keyboard.
uint64_t KEYBOARD_BUTTONS_PRESSED[4];
uint64_t KEYBOARD_BUTTONS_HELD[4];

//Which actions are ocurring or can occur based on which Xbox Controller Buttons are pressed, released, or held down.
uint64_t ACTIONS_HELD_CONTROLLER = 0;
uint64_t ACTIONS_PRESSED_CONTROLLER = 0;
uint64_t ACTIONS_RELEASED_CONTROLLER = 0;

//Which actions are ocurring or can occur based on which Mouse or Keyboard Buttons are pressed, released, or held down.
uint64_t ACTIONS_HELD_PC_INPUTS = 0;
uint64_t ACTIONS_PRESSED_PC_INPUTS = 0;
uint64_t ACTIONS_RELEASED_PC_INPUTS = 0;

//Storage for if an Button is Pressed or Released on Xbox Controller
uint16_t XBOX_BUTTONS_PRESSED = 0;
uint16_t XBOX_BUTTONS_HELD = 0;

uint16_t KeyboardConfiguration[64] = { 0 };
uint8_t ControllerConfigurations[64] = { 0 };

constexpr uint8_t UPDATE_FLAGS_BUTTONS_CHANGED = 0x1;
constexpr uint8_t UPDATE_FLAGS_MOUSE_MOVED = 0x2;
constexpr uint8_t UPDATE_FLAGS_MOUSE_WHEEL = 0x4;
constexpr uint8_t UPDATE_FLAGS_TEXTBOX_TYPING = 0x8;

uint8_t UpdateFlags;

void XGameInput::InitializeDefaultConfigurations()
{
	KeyboardConfiguration[static_cast<uint8_t>(InputActions::MOVE_FORWARD)] = 'w';
	KeyboardConfiguration[static_cast<uint8_t>(InputActions::MOVE_LEFT)] = 'a';
	KeyboardConfiguration[static_cast<uint8_t>(InputActions::MOVE_BACKWARD)] = 's';
	KeyboardConfiguration[static_cast<uint8_t>(InputActions::MOVE_RIGHT)] = 'd';

	KeyboardConfiguration[static_cast<uint8_t>(InputActions::SPRINT)] = static_cast<char>(SHIFT_PRESSED);
	KeyboardConfiguration[static_cast<uint8_t>(InputActions::HOLD_LOOK)] = 'g';
	KeyboardConfiguration[static_cast<uint8_t>(InputActions::CROUCHING)] = static_cast<char>(LEFT_CTRL_PRESSED);
	KeyboardConfiguration[static_cast<uint8_t>(InputActions::JUMPING)] = ' ';

	ControllerConfigurations[static_cast<uint8_t>(InputActions::SPRINT)] = static_cast<uint8_t>(XboxControllerButtonIndexes::LEFT_STICK_CLICK);
	ControllerConfigurations[static_cast<uint8_t>(InputActions::HOLD_LOOK)] = static_cast<uint8_t>(XboxControllerButtonIndexes::LEFT_BUMPER);
	ControllerConfigurations[static_cast<uint8_t>(InputActions::CROUCHING)] = static_cast<uint8_t>(XboxControllerButtonIndexes::B_BUTTON);
	ControllerConfigurations[static_cast<uint8_t>(InputActions::JUMPING)] = static_cast<uint8_t>(XboxControllerButtonIndexes::A_BUTTON);
}

bool XGameInput::LoadController()
{
	// Capture the state of the gamepad before updating it
	const uint16_t LastGamepadState = CurrentGamepadState.Gamepad.wButtons;

	// Update the gamepad state
	if (XInputGetState(0, &CurrentGamepadState) == ERROR_SUCCESS)
	{
		const uint16_t Current = CurrentGamepadState.Gamepad.wButtons;

		if (Current != XBOX_BUTTONS_HELD)
		{
			//wchar_t Buffer[64] = { 0 };
			//swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"Button Value: %d\n", Current);
			//OutputDebugString(Buffer);

			const uint16_t HeldButtons = LastGamepadState & Current;

			XBOX_BUTTONS_HELD = HeldButtons;
			XBOX_BUTTONS_PRESSED = Current ^ HeldButtons;

			const uint16_t XBOX_BUTTONS_RELEASED = LastGamepadState ^ HeldButtons;

			uint16_t HELD = 0x0;
			uint16_t PRESSED = 0x0;
			uint16_t RELEASED = 0x0;

			for (int CurIndex = 4; CurIndex < static_cast<int>(InputActions::MAX); ++CurIndex)
			{
				const uint8_t ActionBitsIndex = ControllerConfigurations[CurIndex];
				HELD |= ((1 << ActionBitsIndex) & XBOX_BUTTONS_HELD);
				PRESSED |= ((1 << ActionBitsIndex) & XBOX_BUTTONS_PRESSED);
				RELEASED |= ((1 << ActionBitsIndex) & XBOX_BUTTONS_RELEASED);
			}

			ACTIONS_HELD_CONTROLLER = HELD;
			ACTIONS_RELEASED_CONTROLLER = RELEASED;
			ACTIONS_PRESSED_CONTROLLER = PRESSED;
		}

		return true;
	}

	return false;
}

void XGameInput::UpdateInputFlags()
{
	if (UpdateFlags & UPDATE_FLAGS_BUTTONS_CHANGED)
	{
/*
		wchar_t Buffer[64] = { 0 };
		swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"Keyboard button action occurred\n");
		OutputDebugString(Buffer);
*/

		UpdateFlags = 0;
	}
}

void XGameInput::StoreRawInputStateChanges(RAWINPUT* &RawInput)
{
	switch (RawInput->header.dwType)
	{
		case RIM_TYPEKEYBOARD:
		{
			const uint8_t VKeyID = static_cast<uint8_t>(RawInput->data.keyboard.VKey);

			if (VKeyID != 0)
			{
				constexpr uint16_t j1 = 260;
				constexpr uint8_t j2 = j1 & 0xFF;
				constexpr uint8_t j3 = static_cast<uint8_t>(j1);


				const uint8_t ArrayIndex = VKeyID >> 6;
				const uint8_t KeyBitIndex = VKeyID & 0x3F;

				const uint64_t PreviousPressedKeysChunk = KEYBOARD_BUTTONS_PRESSED[ArrayIndex];
				const uint64_t PreviousHeldKeysChunk = KEYBOARD_BUTTONS_HELD[ArrayIndex];

				//INPUT_BUTTON_STATES[ArrayIndex] |= (1ULL << KeyBitIndex);
				//INPUT_BUTTON_STATES[ArrayIndex] ^= (static_cast<uint64_t>(RawInput->data.keyboard.Flags) << KeyBitIndex);

				//wchar_t Buffer[64] = { 0 };
				//swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"Keyboard States [Hold=%s,Press=%s,Release=%s]: \n", KeyIsHeldAction ? L"TRUE" : L"FALSE", KeyIsPressedAction ? L"TRUE" : L"FALSE", KeyIsReleasedAction ? L"TRUE" : L"FALSE");
				//OutputDebugString(Buffer);

				//if (CurrentKeyStatePressed != KeyIsHeldAction)
				//{
				//	UpdateFlags |= UPDATE_FLAGS_BUTTONS_CHANGED;
				//}
			}
			break;
		}
		
		case RIM_TYPEMOUSE:
		{
			const uint32_t RawInputButtonState = RawInput->data.mouse.usButtonFlags;

			if (RawInputButtonState != 0)
			{
				// Isolate the lower 5 bits (0 to 31). Used for next stages.
				uint8_t CurrentButtonState = 0;// INPUT_BUTTON_STATES[4] & 0x3F;

				const uint8_t OldButtonState = CurrentButtonState;

				//tempstate - Could be 1 (left pressed) , 2 (right pressed) , 4 (middle pressed) , 3 (left and right pressed) , 5 (left and middle pressed), 7 (left, right, middle pressed)
				//mouse_buttons_state - Could be 1 (left pressed) , 2 (left released), 4 (right pressed), 8 (right released) , 16 (middle pressed), 32 (middle released) , then combo variants up to 63.

				//Set's any new buttons to pressed if not already pressed based on the new information.
				CurrentButtonState |= RawInputButtonState & 0x1;
				CurrentButtonState |= (RawInputButtonState & 0x4) >> 1;
				CurrentButtonState |= (RawInputButtonState & 0x10) >> 2;
				CurrentButtonState |= (RawInputButtonState & 0x40) >> 3;
				CurrentButtonState |= (RawInputButtonState & 0x100) >> 4;

				//Then we build our release mask which will contain the Released Buttons this raw input read.
				uint8_t ReleaseMask = (RawInputButtonState & 0x2) >> 1;
				ReleaseMask |= (RawInputButtonState & 0x8) >> 2;
				ReleaseMask |= (RawInputButtonState & 0x20) >> 3;
				ReleaseMask |= (RawInputButtonState & 0x80) >> 4;
				ReleaseMask |= (RawInputButtonState & 0x200) >> 5;

				CurrentButtonState &= ~ReleaseMask;

				//Only update the cached bits if we need to.
				if (CurrentButtonState != OldButtonState)
				{
					//INPUT_BUTTON_STATES[4] &= 0xFFFFFFFFFFFFFF00;
					//INPUT_BUTTON_STATES[4] |= CurrentButtonState;
				}

				//NOTUSED YET
				int CurrentMouseWheel = 0;
				if (RawInputButtonState & RI_MOUSE_WHEEL)
				{
					int8_t WheelValue = static_cast<int8_t>(RawInput->data.mouse.usButtonData);
					CurrentMouseWheel += WheelValue / WHEEL_DELTA;
				}

				//wchar_t buffer[64];
				//swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"CurrentButtonState: %d, CurrentMouseWheel: %d\n", CurrentButtonState, CurrentMouseWheel);
				//OutputDebugStringW(buffer);
			}
			break;
		}
	}
}

uint16_t XGameInput::GetControllerButtonsPressed(uint16_t ButtonValues)
{
	return XBOX_BUTTONS_PRESSED & ButtonValues;
}

int16_t XGameInput::GetLeftStickX()
{
	return CurrentGamepadState.Gamepad.sThumbLX;
}

int16_t XGameInput::GetLeftStickY()
{
	return CurrentGamepadState.Gamepad.sThumbLY;
}

int16_t XGameInput::GetRightStickX()
{
	return CurrentGamepadState.Gamepad.sThumbRX;
}

int16_t XGameInput::GetRightStickY()
{
	return CurrentGamepadState.Gamepad.sThumbRY;
}

bool XGameInput::IsControllerActionHeld(InputActions Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & ACTIONS_HELD_CONTROLLER;
}

bool XGameInput::IsControllerActionPressed(InputActions Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & ACTIONS_PRESSED_CONTROLLER;
}

bool XGameInput::IsControllerActionReleased(InputActions Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & ACTIONS_RELEASED_CONTROLLER;
}
