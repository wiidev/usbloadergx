/****************************************************************************
 * libwiigui Template
 * Tantric 2009
 * Cyan 2015
 *
 * input.cpp
 * Wii/GameCube controller management
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ogcsys.h>
#include <unistd.h>
#include <wiiuse/wpad.h>
#include "libs/libdrc/wiidrc.h"

#include "menu.h"
#include "video.h"
#include "input.h"
#include "GUI/gui.h"
#include "sys.h"
#include "gecko.h"

int rumbleRequest[4] = { 0, 0, 0, 0 };
GuiTrigger userInput[4];
static int rumbleCount[4] = { 0, 0, 0, 0 };
extern bool isWiiVC; // in sys.cpp

/****************************************************************************
 * UpdatePads
 *
 * called by postRetraceCallback in InitGCVideo - scans gcpad and wpad
 ***************************************************************************/
void UpdatePads()
{
	WPAD_ScanPads();
	PAD_ScanPads();
	
	for (int i = 3; i >= 0; i--)
	{
		memcpy(&userInput[i].wpad, WPAD_Data(i), sizeof(WPADData));
		userInput[i].chan = i;
		userInput[i].pad.btns_d = PAD_ButtonsDown(i);
		userInput[i].pad.btns_u = PAD_ButtonsUp(i);
		userInput[i].pad.btns_h = PAD_ButtonsHeld(i);
		userInput[i].pad.stickX = PAD_StickX(i);
		userInput[i].pad.stickY = PAD_StickY(i);
		userInput[i].pad.substickX = PAD_SubStickX(i);
		userInput[i].pad.substickY = PAD_SubStickY(i);
		userInput[i].pad.triggerL = PAD_TriggerL(i);
		userInput[i].pad.triggerR = PAD_TriggerR(i);

		if (Settings.rumble == ON) DoRumble(i);

		if(userInput[i].wpad.exp.type == WPAD_EXP_NUNCHUK)
		{
			if((userInput[i].wpad.btns_h & WPAD_NUNCHUK_BUTTON_Z) && (userInput[i].wpad.btns_d & WPAD_NUNCHUK_BUTTON_C))
 				ScreenShot();
		}
		if((userInput[i].pad.btns_h & PAD_TRIGGER_R) && (userInput[i].pad.btns_d & PAD_TRIGGER_Z))
 			ScreenShot();
	}

	// WiiU gamepad (DRC) when using WiiVC injected WiiU channels
	// Copy the drc state to Gamecube pad state
	if(WiiDRC_Inited() && WiiDRC_Connected())
	{
		WiiDRC_ScanPads();
		
		// DRC buttons state written to gamecube pad data
		userInput[0].pad.btns_d |= wiidrc_to_pad(WiiDRC_ButtonsDown());
		userInput[0].pad.btns_u |= wiidrc_to_pad(WiiDRC_ButtonsUp());
		userInput[0].pad.btns_h |= wiidrc_to_pad(WiiDRC_ButtonsHeld());
		// DRC stick state written to gamecube pad data
		userInput[0].pad.stickX    = WiiDRC_lStickX();
		userInput[0].pad.stickY    = WiiDRC_lStickY();
		userInput[0].pad.substickX = WiiDRC_rStickX();
		userInput[0].pad.substickY = WiiDRC_rStickY();
	}
}

/****************************************************************************
 * ScreensaverTime
 ***************************************************************************/
static inline u32 ScreensaverTime(int setting)
{
	switch (setting)
	{
		case 0:
			return 0xFFFFFF;
		case 1:
			return 180;
		case 2:
			return 300;
		case 3:
			return 600;
		case 4:
			return 1200;
		case 5:
			return 1800;
		case 6:
			return 3600;
		default:
			break;
	}

	return 0xFFFFFF;
}

/****************************************************************************
 * SetWPADTimeout
 ***************************************************************************/
void SetWPADTimeout()
{
	WPAD_SetIdleTimeout(ScreensaverTime(Settings.screensaver));
}

/****************************************************************************
 * ControlActivityTimeOut
 ***************************************************************************/
bool ControlActivityTimeout(void)
{
	u32 minTime = 0xFFFFFF;
	for(int i = 0; i < 3; ++i)
		if(pointer[i]->getLastActivCounter() < minTime)
			minTime = pointer[i]->getLastActivCounter();

	// not very accurate but it's not required here
	return (minTime/(Settings.PAL50 ? 50 : 60) > ScreensaverTime(Settings.screensaver));
}
/****************************************************************************
 * SetupPads
 *
 * Sets up userInput triggers for use
 ***************************************************************************/
void SetupPads()
{
	PAD_Init();
	WPAD_Init();
	
	// check WiiVC to init WiiU gamepad 
	WiiDRC_Init();
	isWiiVC = WiiDRC_Inited();
	

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

	for (int i = 0; i < 4; i++)
	{
		userInput[i].chan = i;
	}

	SetWPADTimeout();
}

/****************************************************************************
 * ShutoffRumble
 ***************************************************************************/
void ShutoffRumble()
{
	for (int i = 0; i < 4; i++)
	{
		WPAD_Rumble(i, 0);
		rumbleCount[i] = 0;
	}
}

/****************************************************************************
 * DoRumble
 ***************************************************************************/
void DoRumble(int i)
{
	if (rumbleRequest[i] && rumbleCount[i] < 3)
	{
		WPAD_Rumble(i, 1); // rumble on
		rumbleCount[i]++;
	}
	else if (rumbleRequest[i])
	{
		rumbleCount[i] = 20;
		rumbleRequest[i] = 0;
	}
	else
	{
		if (rumbleCount[i]) rumbleCount[i]--;
		WPAD_Rumble(i, 0); // rumble off
	}
}

/****************************************************************************
 * WiiDRC to WPAD
 * 
 * Sets WPAD button state based on WiiDRC (WiiU gamepad in WiiVC) pressed buttons.
 ***************************************************************************/
u32 wiidrc_to_wpad(u32 btns) {
	u32 ret = 0;

	if(btns & WIIDRC_BUTTON_LEFT)
		ret |= WPAD_BUTTON_LEFT;
	if(btns & WIIDRC_BUTTON_RIGHT)
		ret |= WPAD_BUTTON_RIGHT;
	if(btns & WIIDRC_BUTTON_UP)
		ret |= WPAD_BUTTON_UP;
	if(btns & WIIDRC_BUTTON_DOWN)
		ret |= WPAD_BUTTON_DOWN;
	if(btns & WIIDRC_BUTTON_A)
		ret |= WPAD_BUTTON_A;
	if(btns & WIIDRC_BUTTON_B)
		ret |= WPAD_BUTTON_B;
	if(btns & WIIDRC_BUTTON_X)
		ret |= WPAD_BUTTON_1;
	if(btns & WIIDRC_BUTTON_Y)
		ret |= WPAD_BUTTON_2;
	if((btns & WIIDRC_BUTTON_L) || (btns & WIIDRC_BUTTON_ZL) || (btns & WIIDRC_BUTTON_MINUS))
		ret |= WPAD_BUTTON_MINUS;
	if((btns & WIIDRC_BUTTON_R) || (btns & WIIDRC_BUTTON_ZR) || (btns & WIIDRC_BUTTON_PLUS))
		ret |= WPAD_BUTTON_PLUS;
	if(btns & WIIDRC_BUTTON_HOME)
		ret |= WPAD_BUTTON_HOME;

	return (ret&0xffff) ;
}

/****************************************************************************
 * WiiDRC to PAD
 * 
 * Sets PAD button state based on WiiDRC (WiiU gamepad in WiiVC) pressed buttons.
 ***************************************************************************/
u32 wiidrc_to_pad(u32 btns) {
	u32 ret = 0;

	if(btns & WIIDRC_BUTTON_LEFT)
		ret |= PAD_BUTTON_LEFT;
	if(btns & WIIDRC_BUTTON_RIGHT)
		ret |= PAD_BUTTON_RIGHT;
	if(btns & WIIDRC_BUTTON_UP)
		ret |= PAD_BUTTON_UP;
	if(btns & WIIDRC_BUTTON_DOWN)
		ret |= PAD_BUTTON_DOWN;
	if(btns & WIIDRC_BUTTON_A)
		ret |= PAD_BUTTON_A;
	if(btns & WIIDRC_BUTTON_B)
		ret |= PAD_BUTTON_B;
	if(btns & WIIDRC_BUTTON_X)
		ret |= PAD_BUTTON_X;
	if(btns & WIIDRC_BUTTON_Y)
		ret |= PAD_BUTTON_Y;
	if((btns & WIIDRC_BUTTON_L) || (btns & WIIDRC_BUTTON_ZL) || (btns & WIIDRC_BUTTON_MINUS))
		ret |= PAD_TRIGGER_L;
	if((btns & WIIDRC_BUTTON_R) || (btns & WIIDRC_BUTTON_ZR) || (btns & WIIDRC_BUTTON_PLUS))
		ret |= PAD_TRIGGER_R;
	if(btns & WIIDRC_BUTTON_HOME)
		ret |= PAD_BUTTON_START;

	return (ret&0xffff) ;
}