#include <stdio.h>
#include <ogcsys.h>
#include <ogc/pad.h>

#include "sys.h"
#include "wpad.h"

/* Constants */
#define MAX_WIIMOTES	4

extern u8 shutdown;

void __Wpad_PowerCallback(s32 chan)
{
	/* Poweroff console */
	shutdown = 1;
}

s32 Wpad_Init(void)
{
	WUPC_Init();
	s32 ret;

	/* Initialize Wiimote subsystem */
	ret = WPAD_Init();
	if (ret < 0) return ret;

	/* Set POWER button callback */
	WPAD_SetPowerButtonCallback(__Wpad_PowerCallback);

	return ret;
}

void Wpad_Disconnect(void)
{
	u32 cnt;

	/* Disconnect Wiimotes */
	for (cnt = 0; cnt < MAX_WIIMOTES; cnt++)
		WPAD_Disconnect(cnt);

	/* Shutdown Wiimote subsystem */
	WUPC_Shutdown();
	WPAD_Shutdown();
}

bool IsWpadConnected()
{
	int i = 0;
	u32 test = 0;
	int notconnected = 0;
	for (i = 0; i < 4; i++)
	{
		if (WPAD_Probe(i, &test) == WPAD_ERR_NO_CONTROLLER)
		{
			notconnected++;
		}
	}
	if (notconnected < 4)
		return true;
	else return false;
}

u32 ButtonsHold(void)
{

	int i;
	u32 buttons = 0;
	WUPC_UpdateButtonStats();
	WPAD_ScanPads();
	PAD_ScanPads();

	for (i = 3; i >= 0; i--)
	{
		buttons |= WUPC_ButtonsHeld(i);
		buttons |= PAD_ButtonsHeld(i);
		buttons |= WPAD_ButtonsHeld(i);
	}
	return buttons;
}

u32 ButtonsPressed(void)
{
	int i;
	u32 buttons = 0;
	WUPC_UpdateButtonStats();
	WPAD_ScanPads();
	PAD_ScanPads();

	for (i = 3; i >= 0; i--)
	{
		buttons |= WUPC_ButtonsDown(i);
		buttons |= PAD_ButtonsDown(i);
		buttons |= WPAD_ButtonsDown(i);
	}
	return buttons;
}
