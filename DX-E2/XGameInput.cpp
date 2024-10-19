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
uint64_t KEYBOARD_BUTTONS_STATE[4] = {0};

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

uint8_t KeyboardConfigurations[64] = { 0 };
uint8_t ControllerConfigurations[64] = { 0 };

constexpr uint8_t UPDATE_FLAGS_BUTTONS_CHANGED = 0x1;
constexpr uint8_t UPDATE_FLAGS_MOUSE_MOVED = 0x2;
constexpr uint8_t UPDATE_FLAGS_MOUSE_WHEEL = 0x4;
constexpr uint8_t UPDATE_FLAGS_TEXTBOX_TYPING = 0x8;

uint8_t UpdateFlags;

void XGameInput::InitializeDefaultConfigurations()
{
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_FORWARD)] = 'W';
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_LEFT)] = 'A';
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_BACKWARD)] = 'S';
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_RIGHT)] = 'D';

	KeyboardConfigurations[static_cast<uint8_t>(InputActions::SPRINT)] = VK_SHIFT;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::HOLD_LOOK)] = 'G';
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::CROUCHING)] = VK_LCONTROL;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::JUMPING)] = VK_SPACE;

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

			const uint16_t CurrentHeldButtons = LastGamepadState & Current;

			XBOX_BUTTONS_HELD = CurrentHeldButtons;
			XBOX_BUTTONS_PRESSED = Current ^ CurrentHeldButtons;

			const uint16_t XBOX_BUTTONS_RELEASED = LastGamepadState ^ CurrentHeldButtons;

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
			//Pause Key Occurred. NumLock also shows "Pause" fixed in a later step.
			if (RawInput->data.keyboard.Flags & RI_KEY_E1)
				return;

			uint32_t ScanCode = RawInput->data.keyboard.MakeCode;
			ScanCode |= (RawInput->data.keyboard.Flags & RI_KEY_E0) << 7;

			//We can eventually wrap this as a setting to enable so players can decide if they care about Num Lock usage.
			if (RawInput->data.keyboard.VKey != 255)
			{
				int NumFlag = GetKeyState(VK_NUMLOCK) & 0x1;
				switch (ScanCode)
				{
					case 69: //Numlock , Always requires +256
						ScanCode |= 0x100;
						break;
					case 71: 
					case 72: 
					case 73: 

					case 75:
					case 76: 
					case 77:

					case 79:
					case 80:
					case 81:

					case 83:
						ScanCode |= NumFlag << 8;
						break;

					//[Numpad Period] the names won't be mapped correctly requires custom naming, but they'll have unique ids.
					case 82:
						ScanCode |= (NumFlag ^ 1) << 8;
						break;

					default:
						break;	
				}
			}

			if (ScanCode != 0)
			{
				wchar_t KeyName[32] = {0};

				// Define the lambda function which modifies KeyName directly
				auto my_lambda = [&KeyName, ScanCode]() {
					switch (ScanCode) 
					{
					default:
						if(!GetKeyNameText(ScanCode << 16, KeyName, sizeof(KeyName)))
						{
							KeyName[0] = NULL;
						}
						break;
					}
				};

				my_lambda();

				if (KeyName[0] != NULL)
				{
					if (RawInput->data.keyboard.Flags & 0x1)
					{
						wchar_t Buffer[256] = { 0 };
						swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"ScanCode: %d, Name: %s, Flag: %d, MakeCode: %d, VKeyID: %d\n", ScanCode, KeyName, RawInput->data.keyboard.Flags, RawInput->data.keyboard.MakeCode, RawInput->data.keyboard.VKey);
						OutputDebugString(Buffer);
					}
				} 
				else 
				{					wchar_t Buffer[256] = { 0 };
					swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"[Type A] - ScanCode: %d, Name: Unknown, Flag: %d, MakeCode: %d, VKeyID: %d\n", ScanCode, RawInput->data.keyboard.Flags, RawInput->data.keyboard.MakeCode, RawInput->data.keyboard.VKey);
					OutputDebugString(Buffer);
				}
				//const uint8_t ArrayIndex = (VKeyID & 0x7F) >> 6;
				//const uint8_t KeyBitIndex = (VKeyID & 0x3F);
				//const uint64_t KeyBitMask = (1ULL << KeyBitIndex);

				//const uint64_t PreviousKeyboardState = KEYBOARD_BUTTONS_STATE[ArrayIndex];

				//uint64_t CurrentKeyboardState = PreviousKeyboardState;

				//CurrentKeyboardState |= KeyBitMask;
				//CurrentKeyboardState ^= (static_cast<uint64_t>(RawInput->data.keyboard.Flags) << KeyBitIndex);

				//uint64_t IsHoldingKeyDown = PreviousKeyboardState & CurrentKeyboardState;

				//KEYBOARD_BUTTONS_STATE[ArrayIndex] = CurrentKeyboardState;
				//uint64_t InitialPress = CurrentKeyboardState ^ IsHoldingKeyDown;
				//uint64_t ReleasedOccurance = PreviousKeyboardState ^ IsHoldingKeyDown;

				//CurrentKeyboardState |= ((CurrentKeyboardState & KeyBitMask) << 0x20);
				//InitialPress |= ((InitialPress & KeyBitMask) << 0x20);
				//ReleasedOccurance |= ((ReleasedOccurance & KeyBitMask) << 0x20);

				//for (int CurIndex = 0; CurIndex < static_cast<int>(InputActions::MAX); ++CurIndex)
				//{
				//	const uint8_t ActionBitsIndex = KeyboardConfigurations[CurIndex];
				//}
			}
			else
			{
				wchar_t Buffer[256] = { 0 };
				swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"[Type B] - ScanCode: %d, Name: Unknown, Flag: %d, MakeCode: %d, VKeyID: %d\n", ScanCode, RawInput->data.keyboard.Flags, RawInput->data.keyboard.MakeCode, RawInput->data.keyboard.VKey);
				OutputDebugString(Buffer);
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
