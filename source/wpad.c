#include <stdio.h>
#include <ogcsys.h>

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
	s32 ret;

	/* Initialize Wiimote subsystem */
	ret = WPAD_Init();
	if (ret < 0)
		return ret;

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
	WPAD_Shutdown();
}

u32 Wpad_GetButtons(void)
{
	u32 buttons = 0, cnt;

	/* Scan pads */
	WPAD_ScanPads();

	/* Get pressed buttons */
	for (cnt = 0; cnt < MAX_WIIMOTES; cnt++)
		buttons |= WPAD_ButtonsDown(cnt);

	return buttons;
}

u32 Wpad_WaitButtons(void)
{
	u32 buttons = 0;

	/* Wait for button pressing */
	while (!buttons) {
		buttons = Wpad_GetButtons();
		VIDEO_WaitVSync();
	}

	return buttons;
}
