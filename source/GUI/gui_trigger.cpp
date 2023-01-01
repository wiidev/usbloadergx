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

/****************************************************************************
 * WPAD_Stick
 *
 * Get X/Y value from Wii Joystick (classic, nunchuk) input
 ***************************************************************************/
s8 GuiTrigger::WPAD_Stick(u8 right, int axis)
{
	struct joystick_t *js = NULL;

	switch (wpad.exp.type)
	{
	case WPAD_EXP_NUNCHUK:
	case WPAD_EXP_GUITARHERO3:
		js = right ? NULL : &wpad.exp.nunchuk.js;
		break;
	case WPAD_EXP_CLASSIC:
		js = right ? &wpad.exp.classic.rjs : &wpad.exp.classic.ljs;
		break;
	default:
		break;
	}

	if (js)
	{
		int pos = axis ? js->pos.y : js->pos.x;
		int min = axis ? js->min.y : js->min.x;
		int max = axis ? js->max.y : js->max.x;
		int center = axis ? js->center.y : js->center.x;

		// Fix bad calibration values (libogc 2.2.0 also fixes this)
		if ((min >= center) || (max <= center))
		{
			if (axis)
			{
				min = js->min.y = 0;
				max = js->max.y = right ? 32 : 64;
				center = js->center.y = right ? 16 : 32;
			}
			else
			{
				min = js->min.x = 0;
				max = js->max.x = right ? 32 : 64;
				center = js->center.x = right ? 16 : 32;
			}
		}

		// Limit values
		if (pos > max)
			return 127;
		if (pos < min)
			return -128;

		// Adjust against center position
		pos -= center;

		// Return interpolated range
		if (pos > 0)
			return (s8)(127.0 * ((float)pos / (float)(max - center)));
		else
			return (s8)(128.0 * ((float)pos / (float)(center - min)));
	}

	return 0;
}

bool GuiTrigger::Left()
{
	u32 wiibtn = WPAD_BUTTON_LEFT;
	if (wpad.exp.type == WPAD_EXP_CLASSIC)
		wiibtn |= WPAD_CLASSIC_BUTTON_LEFT;

	if (((wpad.btns_d | wpad.btns_h) & wiibtn) || ((pad.btns_d | pad.btns_h) & PAD_BUTTON_LEFT))
	{
		if ((wpad.btns_d & wiibtn) || (pad.btns_d & PAD_BUTTON_LEFT))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if (--scrollDelay <= 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
	}
	return false;
}

bool GuiTrigger::Right()
{
	u32 wiibtn = WPAD_BUTTON_RIGHT;
	if (wpad.exp.type == WPAD_EXP_CLASSIC)
		wiibtn |= WPAD_CLASSIC_BUTTON_RIGHT;

	if (((wpad.btns_d | wpad.btns_h) & wiibtn) || ((pad.btns_d | pad.btns_h) & PAD_BUTTON_RIGHT))
	{
		if ((wpad.btns_d & wiibtn) || (pad.btns_d & PAD_BUTTON_RIGHT))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if (--scrollDelay <= 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
	}
	return false;
}

bool GuiTrigger::Up()
{
	u32 wiibtn = WPAD_BUTTON_UP;
	if (wpad.exp.type == WPAD_EXP_CLASSIC || wpad.exp.type == WPAD_EXP_GUITARHERO3)
		wiibtn |= WPAD_CLASSIC_BUTTON_UP;

	if (((wpad.btns_d | wpad.btns_h) & wiibtn) || ((pad.btns_d | pad.btns_h) & PAD_BUTTON_UP))
	{
		if ((wpad.btns_d & wiibtn) || (pad.btns_d & PAD_BUTTON_UP))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if (--scrollDelay <= 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
	}
	return false;
}

bool GuiTrigger::Down()
{
	u32 wiibtn = WPAD_BUTTON_DOWN;
	if (wpad.exp.type == WPAD_EXP_CLASSIC || wpad.exp.type == WPAD_EXP_GUITARHERO3)
		wiibtn |= WPAD_CLASSIC_BUTTON_DOWN;

	if (((wpad.btns_d | wpad.btns_h) & wiibtn) || ((pad.btns_d | pad.btns_h) & PAD_BUTTON_DOWN))
	{
		if ((wpad.btns_d & wiibtn) || (pad.btns_d & PAD_BUTTON_DOWN))
		{
			scrollDelay = SCROLL_INITIAL_DELAY; // reset scroll delay.
			return true;
		}
		else if (--scrollDelay <= 0)
		{
			scrollDelay = SCROLL_LOOP_DELAY;
			return true;
		}
	}
	return false;
}
