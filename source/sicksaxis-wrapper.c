/*
// A simple wrapper for libsicksaxis, to make it resemble WPAD/PAD more closely.
// Written by daxtsu/thedax. I'm releasing this code into the public domain, so do whatever you want with it.
//
// 2015-01 Cyan
// Added Buttons state Up and Held.
//
*/
#include <sicksaxis.h>
#include "sicksaxis-wrapper.h"

static DS3 first;
static bool psPressed = false;

static unsigned int ButtonsUp;
static unsigned int ButtonsDown;
static unsigned int ButtonsHeld;

bool DS3_Init()
{
	USB_Initialize();

	if (ss_init() < 0)
	{
		return false;
	}

	ss_initialize(&first);

	return true;
}

void DS3_Rumble()
{
	if (first.connected && psPressed)
	{
		ss_set_rumble(&first, 2, 255, 2, 255);
	}
}

void DS3_Cleanup()
{
	psPressed = false;
	ss_close(&first);
	USB_Deinitialize();
}

unsigned int DS3_ButtonsDown()
{
	if (!ss_is_connected(&first) || !psPressed)
		return 0;

	DS3 *controller;
	controller = &first;

	unsigned int pressed = 0;
	
	pressed |= controller->pad.buttons.PS ? DS3_BUTTON_PS : 0;
	pressed |= controller->pad.buttons.start ? DS3_BUTTON_START : 0;
	pressed |= controller->pad.buttons.select ? DS3_BUTTON_SELECT : 0;
	pressed |= controller->pad.buttons.triangle ? DS3_BUTTON_TRIANGLE : 0;
	pressed |= controller->pad.buttons.circle ? DS3_BUTTON_CIRCLE : 0;
	pressed |= controller->pad.buttons.cross ? DS3_BUTTON_CROSS : 0;
	pressed |= controller->pad.buttons.square ? DS3_BUTTON_SQUARE : 0;
	pressed |= controller->pad.buttons.up ? DS3_BUTTON_UP : 0;
	pressed |= controller->pad.buttons.right ? DS3_BUTTON_RIGHT : 0;
	pressed |= controller->pad.buttons.down ? DS3_BUTTON_DOWN : 0;
	pressed |= controller->pad.buttons.left ? DS3_BUTTON_LEFT : 0;
	pressed |= controller->pad.buttons.L1 ? DS3_BUTTON_L1 : 0;
	pressed |= controller->pad.buttons.L2 ? DS3_BUTTON_L2 : 0;
//	pressed |= controller->pad.buttons.L3 ? DS3_BUTTON_L3 : 0;
	pressed |= controller->pad.buttons.R1 ? DS3_BUTTON_R1 : 0;
	pressed |= controller->pad.buttons.R2 ? DS3_BUTTON_R2 : 0;
//	pressed |= controller->pad.buttons.R3 ? DS3_BUTTON_R3 : 0;

	ButtonsUp = ButtonsHeld & ~pressed;
	ButtonsDown = ~ButtonsHeld & pressed;
	ButtonsHeld = pressed;
			
	return ButtonsDown;
}

unsigned int DS3_ButtonsHeld()
{
	return ButtonsHeld;
}

unsigned int DS3_ButtonsUp()
{
	return ButtonsUp;
}

bool DS3_Connected()
{
	return first.connected > 0 && psPressed;
}

void DS3_ScanPads()
{
	if (!ss_is_connected(&first))
	{
		psPressed = false;
		ss_initialize(&first);
		if (ss_open(&first) > 0)
		{
			ss_start_reading(&first);
			ss_set_led(&first, 0);
		}
	}
	else if (first.pad.buttons.PS && !psPressed)
	{
		psPressed = true;
		ss_set_led(&first, 1);
	}
}

int DS3_StickX()
{
	return psPressed? first.pad.left_analog.x - 128 : 0;
}

int DS3_SubStickX()
{
	return psPressed? first.pad.right_analog.x - 128 : 0;
}

int DS3_StickY()
{
	return psPressed? 127 - first.pad.left_analog.y : 0;
}        

int DS3_SubStickY()
{
	return psPressed? 127 - first.pad.right_analog.y : 0;
}
