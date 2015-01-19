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
#include <wupc/wupc.h>

#include "menu.h"
#include "video.h"
#include "input.h"
#include "GUI/gui.h"
#include "sys.h"
#include "gecko.h"

int rumbleRequest[4] = { 0, 0, 0, 0 };
GuiTrigger userInput[4];
static int rumbleCount[4] = { 0, 0, 0, 0 };

/****************************************************************************
 * UpdatePads
 *
 * called by postRetraceCallback in InitGCVideo - scans gcpad and wpad
 ***************************************************************************/
void UpdatePads()
{
	WUPC_UpdateButtonStats();
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
		
		
		// WiiU Pro Controller
		userInput[i].wupcdata.btns_d = WUPC_ButtonsDown(i);
		userInput[i].wupcdata.btns_u = WUPC_ButtonsUp(i);
		userInput[i].wupcdata.btns_h = WUPC_ButtonsHeld(i);
		userInput[i].wupcdata.stickX = WUPC_lStickX(i);
		userInput[i].wupcdata.stickY = WUPC_lStickY(i);
		userInput[i].wupcdata.substickX = WUPC_rStickX(i);
		userInput[i].wupcdata.substickY = WUPC_rStickY(i);
		// Don't use only held to disconnect, on reconnect the pad sends last held state for a short time.
		if((WUPC_ButtonsHeld(i) & WUPC_EXTRA_BUTTON_RSTICK && WUPC_ButtonsDown(i) & WUPC_EXTRA_BUTTON_LSTICK) // R3+L3
		 ||(WUPC_ButtonsHeld(i) & WUPC_EXTRA_BUTTON_LSTICK && WUPC_ButtonsDown(i) & WUPC_EXTRA_BUTTON_RSTICK))
			WUPC_Disconnect(i);
		
		
		if (Settings.rumble == ON) DoRumble(i);

		if(userInput[i].wpad.exp.type == WPAD_EXP_NUNCHUK)
		{
			if((userInput[i].wpad.btns_h & WPAD_NUNCHUK_BUTTON_Z) && (userInput[i].wpad.btns_d & WPAD_NUNCHUK_BUTTON_C))
 				ScreenShot();
		}
		if((userInput[i].pad.btns_h & PAD_TRIGGER_R) && (userInput[i].pad.btns_d & PAD_TRIGGER_Z))
 			ScreenShot();
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
	WUPC_Init();
	PAD_Init();
	WPAD_Init();

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
		WUPC_Rumble(i, 0);
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
		WUPC_Rumble(i, 1);
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
		WUPC_Rumble(i, 0);
	}
}

