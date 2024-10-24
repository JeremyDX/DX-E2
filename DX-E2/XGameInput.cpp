#include "XGameInput.h"

#include "GameTime.h"
#include "Engine.h"
#include <cstdint>
#include <stdio.h>

XINPUT_STATE XboxGamepadState = { };

//Storage for if a Mouse or Keyboard Button is Pressed or Released Currently on Keyboard.
//Raw Input scancodes can range from 0 to 383.
uint64_t KEYBOARD_BUTTONS_STATE[6] = {0};

//Each Configuration Can store 2 Buttons Per Action, ScanKeys exceed 8bit values to 384, Controller will also offer Combo w/ DPad ability.
uint32_t KeyboardConfigurations[64] = { 0 };
uint32_t ControllerConfigurations[64] = { 0 };

//Action Bits will be true if the button to initate it was ever pressed. (Resets on update)
uint64_t GameActionsInitiatedStorage = 0;
//Action Bits will be true if the button is still being held without a Stop action occurring.
uint64_t GameActionsAreActiveStorage = 0;
//Action Bits will be true if the button to end this action was ever performed. (Resets on update)
uint64_t GameActionsHaveEndedStorage = 0;

uint64_t ControllerGameActionsAreToggled = 0;
uint64_t KeyboardGameActionsAreToggled = 0;

uint64_t XGameInput::MouseCalls = 0;

uint8_t MenuActionFlags = 0x0;

constexpr int GetScanCodeIDCompileTime(const uint8_t VKeyId)
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
	case '~':
		return 41;

	case VK_CONTROL:
		return 29;
	case VK_SPACE:
		return 57;
	case VK_RETURN:
		return 28;
	case VK_BACK:
		return 14;
	case VK_LSHIFT:
		return 42;
	case VK_RSHIFT:
		return 54;
	case VK_TAB:
		return 15;
	
	default:
		return 0; // or some other appropriate value
	}
}

constexpr int SCAN_CODE_W = GetScanCodeIDCompileTime('W');
constexpr int SCAN_CODE_A = GetScanCodeIDCompileTime('A');
constexpr int SCAN_CODE_S = GetScanCodeIDCompileTime('S');
constexpr int SCAN_CODE_D = GetScanCodeIDCompileTime('D');
constexpr int SCAN_CODE_G = GetScanCodeIDCompileTime('G');
constexpr int SCAN_CODE_TILDE = GetScanCodeIDCompileTime('~');

constexpr int SCAN_CODE_CONTROL_LEFT = GetScanCodeIDCompileTime(VK_CONTROL);
constexpr int SCAN_CODE_SPACE = GetScanCodeIDCompileTime(VK_SPACE);
constexpr int SCAN_CODE_ENTER = GetScanCodeIDCompileTime(VK_RETURN);
constexpr int SCAN_CODE_BACKSPACE = GetScanCodeIDCompileTime(VK_BACK);
constexpr int SCAN_CODE_SHIFT_LEFT = GetScanCodeIDCompileTime(VK_LSHIFT);
constexpr int SCAN_CODE_SHIFT_RIGHT = GetScanCodeIDCompileTime(VK_RSHIFT);
constexpr int SCAN_CODE_TAB = GetScanCodeIDCompileTime(VK_TAB);

void XGameInput::InitializeDefaultConfigurations()
{ 
	//KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::MOVE_FORWARD)] = ScanW;
	//KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::MOVE_LEFT)] = ScanA;
	//KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::MOVE_BACKWARD)] = ScanS;
	//KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::MOVE_RIGHT)] = ScanD;

	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::HOLD_LOOK)] = SCAN_CODE_G;
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::SPRINT)] = SCAN_CODE_SHIFT_LEFT;
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::CROUCHING)] = SCAN_CODE_CONTROL_LEFT;
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::JUMPING)] = SCAN_CODE_SPACE;

	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::SELECTION_CONFIRM_BUTTON)] = SCAN_CODE_SPACE;
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::BACK_OR_CANCEL_BUTTON)] = SCAN_CODE_BACKSPACE;
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::CHANGE_TAB_PREVIOUS)] = SCAN_CODE_TILDE;  // SHIFT LEFT, LEFT BUMPER, LEFT TRIGGER.
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::CHANGE_TAB_NEXT)] = SCAN_CODE_TAB;  // SHIFT RIGHT, TAB, RIGHT BUMPER, RIGHT TRIGGER.
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::CHANGE_SELECTION_PREVIOUS)] = SCAN_CODE_A;  // A, ARROW_LEFT, DPAD_LEFT, Joystick's Left.
	KeyboardConfigurations[static_cast<uint8_t>(GameInputActionsEnum::CHANGE_SELECTION_NEXT)] = SCAN_CODE_D;   // D, ARROW_RIGHT, DPAD_RIGHT, Joystick's Right.

	ControllerConfigurations[static_cast<uint8_t>(GameInputActionsEnum::SPRINT)] = XboxControllerButtonIndexes::LEFT_STICK_CLICK | (XboxControllerButtonIndexes::LEFT_STICK_CLICK << 8);
	ControllerConfigurations[static_cast<uint8_t>(GameInputActionsEnum::HOLD_LOOK)] = XboxControllerButtonIndexes::LEFT_BUMPER | (XboxControllerButtonIndexes::LEFT_BUMPER << 8);
	ControllerConfigurations[static_cast<uint8_t>(GameInputActionsEnum::CROUCHING)] = XboxControllerButtonIndexes::B_BUTTON | (XboxControllerButtonIndexes::B_BUTTON << 8);
	ControllerConfigurations[static_cast<uint8_t>(GameInputActionsEnum::JUMPING)] = XboxControllerButtonIndexes::A_BUTTON | (XboxControllerButtonIndexes::A_BUTTON << 8);
}

bool XGameInput::LoadAndProcessXboxInputChanges()
{
	if (GameTime::IsHzBasedUpdateBlocked())
	{
		return false;
	}

	// Capture the state of the gamepad before updating it
	const uint16_t LastGamepadButtons = XboxGamepadState.Gamepad.wButtons;

	if (XInputGetState(0, &XboxGamepadState) == ERROR_SUCCESS)
	{
		const uint16_t CurrentGamepadButtons = XboxGamepadState.Gamepad.wButtons;

		if (CurrentGamepadButtons != LastGamepadButtons)
		{
			wchar_t Buffer[64] = { 0 };
			swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"Xbox Buttons Pressed: %d\n", CurrentGamepadButtons);
			OutputDebugString(Buffer);

			int BUTTONS_HELD = LastGamepadButtons & CurrentGamepadButtons;
			int BUTTONS_RELEASED = LastGamepadButtons ^ BUTTONS_HELD;
			int BUTTONS_PRESSED = CurrentGamepadButtons ^ BUTTONS_HELD;

			//This is used for Menu Actions to suggest that a button has occurred and it's ANY BUTTON!
			MenuActionFlags |= BUTTONS_PRESSED > 0;

			for (int CurIndex = 0; CurIndex < static_cast<int>(GameInputActionsEnum::MAX); ++CurIndex)
			{ 
				int PackedButtonMappings = ControllerConfigurations[CurIndex];

				uint8_t Key1 = static_cast<uint8_t>(PackedButtonMappings);
				uint8_t Key2 = static_cast<uint8_t>(PackedButtonMappings >> 0x8);
				uint8_t Key3 = static_cast<uint8_t>(PackedButtonMappings >> 0x10);
				uint8_t Key4 = static_cast<uint8_t>(PackedButtonMappings >> 0x18);

				int IsKeysPressedCombo1 = (BUTTONS_PRESSED & (1 << Key1)) >> Key1 & (BUTTONS_PRESSED & (1 << Key2)) >> Key2;
				int IsKeysPressedCombo2 = (BUTTONS_PRESSED & (1 << Key3)) >> Key3 & (BUTTONS_PRESSED & (1 << Key4)) >> Key4;

				int IsKeysReleasedCombo1 = (BUTTONS_RELEASED & (1 << Key1)) >> Key1 & (BUTTONS_RELEASED & (1 << Key2)) >> Key2;
				int IsKeysReleasedCombo2 = (BUTTONS_RELEASED & (1 << Key3)) >> Key3 & (BUTTONS_RELEASED & (1 << Key4)) >> Key4;

				int ActionPressedMask = ((IsKeysPressedCombo1 | IsKeysPressedCombo2) << CurIndex);
				int ActionReleasedMask = (IsKeysReleasedCombo1 | IsKeysReleasedCombo2) << CurIndex;

				if (ControllerGameActionsAreToggled & (1ULL << CurIndex))
				{
					GameActionsHaveEndedStorage |= (GameActionsAreActiveStorage & ActionPressedMask);
					GameActionsAreActiveStorage ^= ActionPressedMask;
					GameActionsInitiatedStorage |= (GameActionsAreActiveStorage & ActionPressedMask);
					continue;
				}

				GameActionsInitiatedStorage |= ActionPressedMask;

				GameActionsAreActiveStorage &= ~ActionReleasedMask;
				GameActionsAreActiveStorage |= ActionPressedMask;

				GameActionsHaveEndedStorage |= ActionReleasedMask;

			}
		}

		return true;
	}

	return false;
}

void XGameInput::GameInputPostProcessing()
{
	//Clear Action Caches for Start/Stop only CurrentlyActive remains.
	GameActionsInitiatedStorage = 0;
	GameActionsHaveEndedStorage = 0;
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

			//Obtain the Current/Base Scan Code from the Keyboards MakeCode.
			uint32_t ScanCode = RawInput->data.keyboard.MakeCode;

			//Invalid Scan Code Somehow??
			if (ScanCode == 0 || ScanCode > 383)
				return;

			//This is the correct way to handle the extended #EO Extension. Add 256 when extended. It's that simple.
			ScanCode |= (CurrentFlag & RI_KEY_E0) << 7;

			if ((CurrentFlag & RI_KEY_E0) == 0x2)
			{
				ScanCode |= 0x100;
			}

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
				//Inverse of KeyReleasedFlag so we have a KeyPressedFlag as well as we need it for other situations.
				const uint64_t KeyPressedFlag = KeyReleasedFlag ^ 1;

				//This is used for Menu Actions to suggest that a button has occurred and it's ANY BUTTON!
				MenuActionFlags |= (0x1 & KeyPressedFlag);

				//Store the current status of our buttons in the main button state cache.
				KEYBOARD_BUTTONS_STATE[ArrayIndex] = CurrentStorageReadState;

				//This is to create Init, Hold, End action Events. Doing a Init/End in same frame causes nothing to occur.
				for (int CurIndex = 0; CurIndex < static_cast<int>(GameInputActionsEnum::MAX); ++CurIndex)
				{
					int KeyConfig1 = KeyboardConfigurations[CurIndex] & 0x1FF;
					int KeyConfig2 = (KeyboardConfigurations[CurIndex] >> 0x9) & 0x1FF;

					if (KeyConfig1 == ScanCode || KeyConfig2 == ScanCode)
					{
						uint64_t KeyPressedMask = KeyPressedFlag << CurIndex;
						uint64_t KeyReleaseMask = KeyReleasedFlag << CurIndex;

						bool Toggle = true;

						//For Buttons that are "Toggle Press" style'd. Init/End only reset on Frame Update. Active Flips Every Press.
						if (KeyboardGameActionsAreToggled & (1ULL << CurIndex))
						{
							GameActionsHaveEndedStorage |= (GameActionsAreActiveStorage & KeyPressedMask);
							GameActionsAreActiveStorage ^= KeyPressedMask;
							GameActionsInitiatedStorage |= (GameActionsAreActiveStorage & KeyPressedMask);
							continue;
						} 

						//For Buttons that are "Hold/Release" style'd. Init/End only reset on Frame Update. Active enabled only on Press/Hold.
						GameActionsInitiatedStorage |= KeyPressedMask;

						GameActionsAreActiveStorage &= ~KeyReleaseMask;
						GameActionsAreActiveStorage |= KeyPressedMask;

						GameActionsHaveEndedStorage |= KeyReleaseMask;

						continue;
					}
				}

				wchar_t KeyName[16] = { 0 };
				GetScanCodeKeyName(KeyName, sizeof(KeyName) / sizeof(wchar_t), ScanCode);

				if (KeyName[0] != NULL)
				{
					wchar_t Buffer[256] = { 0 };
					swprintf_s(Buffer, sizeof(Buffer) / sizeof(Buffer[0]), L"ScanCode: %d, Name: %s, Flag: %d, MakeCode: %d, VKeyID: %d\n", ScanCode, KeyName, RawInput->data.keyboard.Flags, RawInput->data.keyboard.MakeCode, RawInput->data.keyboard.VKey);
					OutputDebugString(Buffer);
				}
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

int16_t XGameInput::GetLeftStickX()
{
	return XboxGamepadState.Gamepad.sThumbLX;
}

int16_t XGameInput::GetLeftStickY()
{
	return XboxGamepadState.Gamepad.sThumbLY;
}

int16_t XGameInput::GetRightStickX()
{
	return XboxGamepadState.Gamepad.sThumbRX;
}

int16_t XGameInput::GetRightStickY()
{
	return XboxGamepadState.Gamepad.sThumbRY;
}

bool XGameInput::ActionWasInitiated(GameInputActionsEnum Action)
{
	if (GameActionsInitiatedStorage != 0x0)
	{
		int junk2 = 0;
	}

	return (1ULL << static_cast<uint8_t>(Action)) & GameActionsInitiatedStorage;
}

bool XGameInput::ActionIsCurrentlyActive(GameInputActionsEnum Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & GameActionsAreActiveStorage;
}

bool XGameInput::ActionHasEnded(GameInputActionsEnum Action)
{
	return (1ULL << static_cast<uint8_t>(Action)) & GameActionsHaveEndedStorage;
}

bool XGameInput::HasFlagSettings(const uint8_t FlagSettings)
{
	return (MenuActionFlags & FlagSettings) == FlagSettings;
}