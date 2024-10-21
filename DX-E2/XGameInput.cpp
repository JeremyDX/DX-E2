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
//Raw Input scancodes can range from 0 to 383.
uint64_t KEYBOARD_BUTTONS_STATE[6] = {0};

//Which actions are ocurring or can occur based on which Xbox Controller Buttons are pressed, released, or held down.
uint64_t ACTIONS_HELD_CONTROLLER = 0;
uint64_t ACTIONS_PRESSED_CONTROLLER = 0;
uint64_t ACTIONS_RELEASED_CONTROLLER = 0;

//Which actions are ocurring or can occur based on which Mouse or Keyboard Buttons are pressed, released, or held down.
uint64_t GameActionsCurrentlyHeld = 0;
uint64_t GameActionsInitiallyPressed = 0;
uint64_t GameActionsFinallyReleased = 0;

uint64_t XGameInput::MouseCalls = 0;

//Storage for if an Button is Pressed or Released on Xbox Controller
uint16_t XBOX_BUTTONS_PRESSED = 0;
uint16_t XBOX_BUTTONS_HELD = 0;

//Each Configuration Can store 2 Buttons Per Action, ScanKeys exceed 8bit values to 384, Controller will also offer Combo w/ DPad ability.
uint32_t KeyboardConfigurations[64] = { 0 };
uint32_t ControllerConfigurations[64] = { 0 };

constexpr uint8_t UPDATE_FLAGS_BUTTONS_CHANGED = 0x1;
constexpr uint8_t UPDATE_FLAGS_MOUSE_MOVED = 0x2;
constexpr uint8_t UPDATE_FLAGS_MOUSE_WHEEL = 0x4;
constexpr uint8_t UPDATE_FLAGS_TEXTBOX_TYPING = 0x8;

uint8_t UpdateFlags;

constexpr int GetScanCodeIDCompileTime(const char VKeyId)
{
	switch (VKeyId)
	{
		case 'W':
			return 17;
		case 'A':
			return 30;
		case 'S':
			return 31;
		case 'D':
			return 32;

		case 'G':
			return 34;

		case VK_CONTROL:
			return 29;
		case VK_SHIFT:
			return 42;
		case VK_SPACE:
			return 57;

		default:
			return 0;
	}
}

void XGameInput::InitializeDefaultConfigurations()
{ 
	constexpr int ScanW = GetScanCodeIDCompileTime('W');
	constexpr int ScanA = GetScanCodeIDCompileTime('A');
	constexpr int ScanS = GetScanCodeIDCompileTime('S');
	constexpr int ScanD = GetScanCodeIDCompileTime('D');
	constexpr int ScanG = GetScanCodeIDCompileTime('G');
		
	constexpr int ScanShift = GetScanCodeIDCompileTime(VK_SHIFT);
	constexpr int ScanControl = GetScanCodeIDCompileTime(VK_CONTROL);
	constexpr int ScanSpace = GetScanCodeIDCompileTime(VK_SPACE);

	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_FORWARD)] = ScanW;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_LEFT)] = ScanA;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_BACKWARD)] = ScanS;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::MOVE_RIGHT)] = ScanD;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::HOLD_LOOK)] = ScanG;

	KeyboardConfigurations[static_cast<uint8_t>(InputActions::SPRINT)] = ScanShift;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::CROUCHING)] = ScanControl;
	KeyboardConfigurations[static_cast<uint8_t>(InputActions::JUMPING)] = ScanSpace;

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

void XGameInput::GameInputPostProcessing()
{
	GameActionsFinallyReleased = 0;
	ACTIONS_RELEASED_CONTROLLER = 0;
}

void GetScanCodeKeyName(wchar_t* KeyName, int Length, int ScanCode)
{
	switch (ScanCode)
	{
		case 83:
			wcscpy_s(KeyName, Length, L"Num .");
			break;

		default:
			GetKeyNameText(ScanCode << 16, KeyName, Length);
			break;
	}
}

void XGameInput::StoreRawInputStateChanges(RAWINPUT* &RawInput)
{
	switch (RawInput->header.dwType)
	{
		case RIM_TYPEKEYBOARD:
		{
			const uint16_t CurrentFlag = RawInput->data.keyboard.Flags;

			//Pause Key Occurred. NumLock also shows "Pause" due to Win10/Driver Issues likely. Fixed in a later step.
			if (CurrentFlag & RI_KEY_E1)
				return;

			//THE PROPER WAY to resolve Extended Flags E0 Value.
			uint32_t ScanCode = RawInput->data.keyboard.MakeCode;

			//Invalid Scan Code Somehow??
			if (ScanCode == 0 || ScanCode > 383)
				return;

			ScanCode |= (CurrentFlag & RI_KEY_E0) << 7;

			//We can eventually wrap this as a setting to enable so players can decide if they care about Num Lock usage.
			//Disabling it would provide Num 0 and Num Delete which aren't available with this system due to Scan Limits of 82/83/338/339
			//We Check for 255 because the real Pause uses 255 VKey and a few other keys as well.
			if (RawInput->data.keyboard.VKey != 255)
			{
				int NumFlag = GetKeyState(VK_NUMLOCK) & 0x1;
				switch (ScanCode)
				{
					case 69: //Numlock , Fixes Num Lock which should be an Extended Key and isn't.
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
						ScanCode |= NumFlag << 8;
						break;

					//[Numpad 0 + .] Resolved when Nums Lock is available. If On we do nothing if Off we OR 256.
					case 82:
					case 83:
						ScanCode |= (NumFlag ^ 1) << 8;
						break;

					default:
						break;	
				}
			}

			const uint8_t ArrayIndex = ScanCode >> 6;
			const uint8_t KeyBitIndex = ScanCode & 0x3F;

			//Last Read State.
			const uint64_t LastStorageReadState = KEYBOARD_BUTTONS_STATE[ArrayIndex];

			//Current State of Key. 0x1 for Released and 0x0 for Pressed.
			const uint64_t KeyReleasedFlag = CurrentFlag & 0x1;

			//Get an updated storage value.
			uint64_t CurrentStorageReadState = LastStorageReadState;
			CurrentStorageReadState |= (1ULL << KeyBitIndex);
			CurrentStorageReadState ^= (KeyReleasedFlag << KeyBitIndex);

			//Update Action Press/Hold Storage if Key Change occurred.
			if (LastStorageReadState != CurrentStorageReadState)
			{
				KEYBOARD_BUTTONS_STATE[ArrayIndex] = CurrentStorageReadState;

				const uint64_t KeyPressedFlag = KeyReleasedFlag ^ 1;

				wchar_t KeyName[16] = {0};
				GetScanCodeKeyName(KeyName, sizeof(KeyName) / sizeof(wchar_t), ScanCode);

				//Builds Current Action States.
				//uint64_t ACTION_PRESSED_FLAGS = 0x0;
				//uint64_t ACTION_HELD_FLAGS = 0x0;
				//uint64_t ACTION_RELEASED_FLAGS = 0x0;

				for (int CurIndex = 0; CurIndex < static_cast<int>(InputActions::MAX); ++CurIndex)
				{
					if (KeyboardConfigurations[CurIndex] == ScanCode)
					{
						const uint64_t KeyPressedMask = KeyPressedFlag << CurIndex;
						const uint64_t KeyReleasedMask = KeyReleasedFlag << CurIndex;

						GameActionsCurrentlyHeld &= ~KeyReleasedMask;
						GameActionsCurrentlyHeld |= KeyPressedMask;

						GameActionsInitiallyPressed = GameActionsCurrentlyHeld;

						GameActionsFinallyReleased &= ~KeyPressedMask;
						GameActionsFinallyReleased |= KeyReleasedMask;
						break;
					}
				}

				if (KeyName[0] != NULL)
				{
					wchar_t Buffer[256] = { 0 };
					swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"ScanCode: %d, Name: %s, Flag: %d, MakeCode: %d, VKeyID: %d\n", ScanCode, KeyName, RawInput->data.keyboard.Flags, RawInput->data.keyboard.MakeCode, RawInput->data.keyboard.VKey);
					OutputDebugString(Buffer);
				}
			} 
			else 
			{
				OutputDebugString(L"Key Held\n");
			}
			break;
		}
		
		case RIM_TYPEMOUSE:
		{
			MouseCalls++;

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

bool XGameInput::IsActionInitiallyPressed(InputActions Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & GameActionsInitiallyPressed;
}

bool XGameInput::IsActionCurrentlyHeld(InputActions Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & GameActionsCurrentlyHeld;
}

bool XGameInput::IsActionFinallyReleased(InputActions Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & GameActionsFinallyReleased;
}
