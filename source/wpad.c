#include <stdio.h>
#include <ogcsys.h>
#include <ogc/pad.h>

#include "sys.h"
#include "wpad.h"

/* Constants */
#define MAX_WIIMOTES    4

extern u8 shutdown;

void __Wpad_PowerCallback( s32 chan )
{
    /* Poweroff console */
    shutdown = 1;
}

void WPad_SetIdleTime( u32 seconds )
{
    /*Set idle time for wiimote*/
    WPAD_SetIdleTimeout( seconds );
}

s32 Wpad_Init( void )
{
    s32 ret;

    /* Initialize Wiimote subsystem */
    ret = WPAD_Init();
    if ( ret < 0 )
        return ret;

    /* Set POWER button callback */
    WPAD_SetPowerButtonCallback( __Wpad_PowerCallback );

    return ret;
}

void Wpad_Disconnect( void )
{
    u32 cnt;

    /* Disconnect Wiimotes */
    for ( cnt = 0; cnt < MAX_WIIMOTES; cnt++ )
        WPAD_Disconnect( cnt );

    /* Shutdown Wiimote subsystem */
    WPAD_Shutdown();
}

bool IsWpadConnected()
{
    int i = 0;
    u32 test = 0;
    int notconnected = 0;
    for ( i = 0; i < 4; i++ )
    {
        if ( WPAD_Probe( i, &test ) == WPAD_ERR_NO_CONTROLLER )
        {
            notconnected++;
        }
    }
    if ( notconnected < 4 )
        return true;
    else
        return false;
}

u32 ButtonsHold( void )
{

    int i;
    u32 buttons = 0;

    WPAD_ScanPads();
    PAD_ScanPads();

    for ( i = 3; i >= 0; i-- )
    {
        buttons |= PAD_ButtonsHeld( i );
        buttons |= WPAD_ButtonsHeld( i );
    }
    return buttons;
}

u32 ButtonsPressed( void )
{

    int i;
    u32 buttons = 0;

    WPAD_ScanPads();
    PAD_ScanPads();

    for ( i = 3; i >= 0; i-- )
    {
        buttons |= PAD_ButtonsDown( i );
        buttons |= WPAD_ButtonsDown( i );
    }
    return buttons;

    /*  Don't remove this commented out code it might be useful for checking which buttons were pressed/hold

        if(buttons & (PAD_BUTTON_LEFT | PAD_BUTTON_RIGHT | PAD_BUTTON_DOWN | PAD_BUTTON_UP
                        | PAD_BUTTON_A | PAD_BUTTON_B | PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_MENU
                        | PAD_BUTTON_START | WPAD_BUTTON_2 | WPAD_BUTTON_1
                        | WPAD_BUTTON_B | WPAD_BUTTON_A | WPAD_BUTTON_MINUS
                        | WPAD_BUTTON_HOME | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT
                        | WPAD_BUTTON_DOWN | WPAD_BUTTON_UP | WPAD_BUTTON_PLUS
                        | WPAD_NUNCHUK_BUTTON_Z | WPAD_NUNCHUK_BUTTON_C
                        | WPAD_CLASSIC_BUTTON_UP | WPAD_CLASSIC_BUTTON_LEFT
                        | WPAD_CLASSIC_BUTTON_ZR | WPAD_CLASSIC_BUTTON_X
                        | WPAD_CLASSIC_BUTTON_A | WPAD_CLASSIC_BUTTON_Y
                        | WPAD_CLASSIC_BUTTON_B | WPAD_CLASSIC_BUTTON_ZL
                        | WPAD_CLASSIC_BUTTON_FULL_R | WPAD_CLASSIC_BUTTON_PLUS
                        | WPAD_CLASSIC_BUTTON_HOME | WPAD_CLASSIC_BUTTON_MINUS
                        | WPAD_CLASSIC_BUTTON_FULL_L | WPAD_CLASSIC_BUTTON_DOWN
                        | WPAD_CLASSIC_BUTTON_RIGHT | WPAD_GUITAR_HERO_3_BUTTON_STRUM_UP
                        | WPAD_GUITAR_HERO_3_BUTTON_YELLOW | WPAD_GUITAR_HERO_3_BUTTON_GREEN
                        | WPAD_GUITAR_HERO_3_BUTTON_BLUE | WPAD_GUITAR_HERO_3_BUTTON_RED
                        | WPAD_GUITAR_HERO_3_BUTTON_ORANGE | WPAD_GUITAR_HERO_3_BUTTON_PLUS
                        | WPAD_GUITAR_HERO_3_BUTTON_MINUS | WPAD_GUITAR_HERO_3_BUTTON_STRUM_DOWN)
          )
    */

}
