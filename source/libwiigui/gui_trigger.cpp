/****************************************************************************
 * libwiigui
 *
 * Tantric 2009
 *
 * gui_trigger.cpp
 *
 * GUI class definitions
 ***************************************************************************/

#include "gui.h"

static int scrollDelay = 0;

/**
 * Constructor for the GuiTrigger class.
 */
GuiTrigger::GuiTrigger()
{
	chan = -1;
	memset(&wpad, 0, sizeof(WPADData));
	memset(&pad, 0, sizeof(PADData));
}

/**
 * Destructor for the GuiTrigger class.
 */
GuiTrigger::~GuiTrigger()
{
}

/**
 * Sets a simple trigger. Requires:
 * - Element is selected
 * - Trigger button is pressed
 */
void GuiTrigger::SetSimpleTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_SIMPLE;
	chan = ch;
	wpad.btns_d = wiibtns;
	pad.btns_d = gcbtns;
}

/**
 * Sets a held trigger. Requires:
 * - Element is selected
 * - Trigger button is pressed and held
 */
void GuiTrigger::SetHeldTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_HELD;
	chan = ch;
	wpad.btns_h = wiibtns;
	pad.btns_h = gcbtns;
}

/**
 * Sets a button trigger. Requires:
 * - Trigger button is pressed
 */
void GuiTrigger::SetButtonOnlyTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_BUTTON_ONLY;
	chan = ch;
	wpad.btns_d = wiibtns;
	pad.btns_d = gcbtns;
}

/**
 * Sets a button trigger. Requires:
 * - Trigger button is pressed
 * - Parent window is in focus
 */
void GuiTrigger::SetButtonOnlyInFocusTrigger(s32 ch, u32 wiibtns, u16 gcbtns)
{
	type = TRIGGER_BUTTON_ONLY_IN_FOCUS;
	chan = ch;
	wpad.btns_d = wiibtns;
	pad.btns_d = gcbtns;
}

/****************************************************************************
 * WPAD_Stick
 *
 * Get X/Y value from Wii Joystick (classic, nunchuk) input
 ***************************************************************************/

s8 GuiTrigger::WPAD_Stick(u8 right, int axis)
{
	#ifdef HW_RVL

	float mag = 0.0;
	float ang = 0.0;

	switch (wpad.exp.type)
	{
		case WPAD_EXP_NUNCHUK:
		case WPAD_EXP_GUITARHERO3:
			if (right == 0)
			{
				mag = wpad.exp.nunchuk.js.mag;
				ang = wpad.exp.nunchuk.js.ang;
			}
			break;

		case WPAD_EXP_CLASSIC:
			if (right == 0)
			{
				mag = wpad.exp.classic.ljs.mag;
				ang = wpad.exp.classic.ljs.ang;
			}
			else
			{
				mag = wpad.exp.classic.rjs.mag;
				ang = wpad.exp.classic.rjs.ang;
			}
			break;

		default:
			break;
	}

	/* calculate x/y value (angle need to be converted into radian) */
	if (mag > 1.0) mag = 1.0;
	else if (mag < -1.0) mag = -1.0;
	double val;

	if(axis == 0) // x-axis
		val = mag * sin((PI * ang)/180.0f);
	else // y-axis
		val = mag * cos((PI * ang)/180.0f);

	return (s8)(val * 128.0f);

	#else
	return 0;
	#endif
}

bool GuiTrigger::Left()
{
	u32 wiibtn = WPAD_BUTTON_LEFT;

	if((wpad.btns_d | wpad.btns_h) & (wiibtn | WPAD_CLASSIC_BUTTON_LEFT)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_LEFT
			|| pad.stickX < -PADCAL
			|| WPAD_Stick(0,0) < -PADCAL)
	{
		if(wpad.btns_d & (wiibtn | WPAD_CLASSIC_BUTTON_LEFT)
			|| pad.btns_d & PAD_BUTTON_LEFT)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Right()
{
	u32 wiibtn = WPAD_BUTTON_RIGHT;

	if((wpad.btns_d | wpad.btns_h) & (wiibtn | WPAD_CLASSIC_BUTTON_RIGHT)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_RIGHT
			|| pad.stickX > PADCAL
			|| WPAD_Stick(0,0) > PADCAL)
	{
		if(wpad.btns_d & (wiibtn | WPAD_CLASSIC_BUTTON_RIGHT)
			|| pad.btns_d & PAD_BUTTON_RIGHT)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Up()
{
	u32 wiibtn = WPAD_BUTTON_UP;

	if((wpad.btns_d | wpad.btns_h) & (wiibtn | WPAD_CLASSIC_BUTTON_UP)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_UP
			|| pad.stickY > PADCAL
			|| WPAD_Stick(0,1) > PADCAL)
	{
		if(wpad.btns_d & (wiibtn | WPAD_CLASSIC_BUTTON_UP)
			|| pad.btns_d & PAD_BUTTON_UP)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}

bool GuiTrigger::Down()
{
	u32 wiibtn = WPAD_BUTTON_DOWN;

	if((wpad.btns_d | wpad.btns_h) & (wiibtn | WPAD_CLASSIC_BUTTON_DOWN)
			|| (pad.btns_d | pad.btns_h) & PAD_BUTTON_DOWN
			|| pad.stickY < -PADCAL
			|| WPAD_Stick(0,1) < -PADCAL)
	{
		if(wpad.btns_d & (wiibtn | WPAD_CLASSIC_BUTTON_DOWN)
			|| pad.btns_d & PAD_BUTTON_DOWN)
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if(scrollDelay == 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
		else
		{
			if(scrollDelay > 0)
				scrollDelay--;
		}
	}
	return false;
}
