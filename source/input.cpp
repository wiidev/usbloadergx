/****************************************************************************
 * libwiigui Template
 * Tantric 2009
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

#include "menu.h"
#include "video.h"
#include "input.h"
#include "libwiigui/gui.h"

int rumbleRequest[4] = {0,0,0,0};
GuiTrigger userInput[4];
static int rumbleCount[4] = {0,0,0,0};

/****************************************************************************
 * UpdatePads
 *
 * called by postRetraceCallback in InitGCVideo - scans gcpad and wpad
 ***************************************************************************/
void UpdatePads()
{
    WPAD_ScanPads();
    PAD_ScanPads();

    for (int i=3; i >= 0; i--)
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

        if(Settings.rumble == RumbleOn)
            DoRumble(i);
    }
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

	// read wiimote accelerometer and IR data
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);

	for(int i=0; i < 4; i++)
	{
		userInput[i].chan = i;
	}
}

/****************************************************************************
 * ShutoffRumble
 ***************************************************************************/

void ShutoffRumble() {
    for (int i=0;i<4;i++) {
        WPAD_Rumble(i, 0);
        rumbleCount[i] = 0;
    }
}

/****************************************************************************
 * DoRumble
 ***************************************************************************/

void DoRumble(int i) {
    if (rumbleRequest[i] && rumbleCount[i] < 3) {
        WPAD_Rumble(i, 1); // rumble on
        rumbleCount[i]++;
    } else if (rumbleRequest[i]) {
        rumbleCount[i] = 20;
        rumbleRequest[i] = 0;
    } else {
        if (rumbleCount[i])
            rumbleCount[i]--;
        WPAD_Rumble(i, 0); // rumble off
    }
}

/****************************************************************************
 * WPAD_Stick
 *
 * Get X/Y value from Wii Joystick (classic, nunchuk) input
 ***************************************************************************/

s8 WPAD_Stick(u8 chan, u8 right, int axis) {
    float mag = 0.0;
    float ang = 0.0;
    WPADData *data = WPAD_Data(chan);

    switch (data->exp.type) {
    case WPAD_EXP_NUNCHUK:
    case WPAD_EXP_GUITARHERO3:
        if (right == 0) {
            mag = data->exp.nunchuk.js.mag;
            ang = data->exp.nunchuk.js.ang;
        }
        break;

    case WPAD_EXP_CLASSIC:
        if (right == 0) {
            mag = data->exp.classic.ljs.mag;
            ang = data->exp.classic.ljs.ang;
        } else {
            mag = data->exp.classic.rjs.mag;
            ang = data->exp.classic.rjs.ang;
        }
        break;

    default:
        break;
    }

    /* calculate x/y value (angle need to be converted into radian) */
    if (mag > 1.0) mag = 1.0;
    else if (mag < -1.0) mag = -1.0;
    double val;

    if (axis == 0) // x-axis
        val = mag * sin((PI * ang)/180.0f);
    else // y-axis
        val = mag * cos((PI * ang)/180.0f);

    return (s8)(val * 128.0f);
}
