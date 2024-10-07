#include "XGameInput.h"

#include "GameTime.h"

#include <cstdint>

XINPUT_STATE CurrentGamepadState = { };

uint64_t TRACKING_BEGIN = 0;

uint16_t BUTTONS_HOLD = 0;
uint16_t BUTTONS_PRESSED = 0;
uint16_t BUTTONS_RELEASED = 0;
uint16_t HOLD_TRACKING = 0;

bool XGameInput::LoadController()
{
	// Capture the state of the gamepad before updating it
	const uint16_t LastGamepadState = CurrentGamepadState.Gamepad.wButtons;

	// Update the gamepad state
	if (XInputGetState(0, &CurrentGamepadState) == ERROR_SUCCESS)
	{
		const uint16_t Current = CurrentGamepadState.Gamepad.wButtons;

		// Calculate button states
		BUTTONS_HOLD = LastGamepadState & Current;
		BUTTONS_RELEASED = LastGamepadState ^ BUTTONS_HOLD;
		BUTTONS_PRESSED = Current ^ BUTTONS_HOLD;

		return true;
	}
	return false;
}

uint64_t XGameInput::GetTrackedHoldTime(uint16_t value)
{
	HOLD_TRACKING = value;
	return GameTime::GetAbsoluteFrameTicks() - TRACKING_BEGIN;
}

void XGameInput::ResetHoldTracking()
{
	HOLD_TRACKING = XBOX_CONTROLLER::ALL_BUTTONS;
	TRACKING_BEGIN = GameTime::GetAbsoluteFrameTicks();
}

XINPUT_GAMEPAD& XGameInput::GamePad()
{
	return CurrentGamepadState.Gamepad;
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

int16_t XGameInput::GetButtonBitSet()
{
	return CurrentGamepadState.Gamepad.wButtons;
}

bool XGameInput::AllOfTheseButtonsArePressed(int value)
{
	return (BUTTONS_PRESSED & value) == value;
}

bool XGameInput::AllOfTheseButtonsAreReleased(int value)
{
	return (BUTTONS_RELEASED & value) == value;
}

bool XGameInput::AllOfTheseButtonsAreHolding(int value)
{
	return (BUTTONS_HOLD & value) == value;
}

uint16_t XGameInput::AnyOfTheseButtonsArePressed(int value)
{
	return BUTTONS_PRESSED & value;
}

uint16_t XGameInput::AnyOfTheseButtonsAreReleased(int value)
{
	return BUTTONS_RELEASED & value;
}

uint16_t XGameInput::AnyOfTheseButtonsAreHolding(int value)
{
	return BUTTONS_HOLD & value;
}

bool XGameInput::AnyButtonPressed()
{
	return BUTTONS_PRESSED != 0;
}

bool XGameInput::AnyButtonReleased()
{
	return BUTTONS_PRESSED != 0;
}

bool XGameInput::AnyButtonHeld()
{
	return BUTTONS_HOLD != 0;
}