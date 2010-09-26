#include <gctypes.h>
#include <ogc/system.h>
#include <wiiuse/wpad.h>

#include "mload/mload.h"
#include "settings/CSettings.h"
#include "utils/ResourceManager.h"
#include "audio.h"
#include "fatmounter.h"
#include "lstub.h"
#include "menu.h"
#include "video.h"

extern char game_partition[6];
extern u8 load_from_fs;

//Wiilight stuff
static vu32 *_wiilight_reg = (u32*) 0xCD0000C0;
void wiilight(int enable) // Toggle wiilight (thanks Bool for wiilight source)
{
    u32 val = (*_wiilight_reg & ~0x20);
    if (enable && Settings.wiilight) val |= 0x20;
    *_wiilight_reg = val;
}

/* Variables */
u8 shutdown = 0;
u8 reset = 0;

void __Sys_ResetCallback(void)
{
    /* Reboot console */
    reset = 1;
}

void __Sys_PowerCallback(void)
{
    /* Poweroff console */
    shutdown = 1;
}

void Sys_Init(void)
{
    /* Initialize video subsytem */
    //VIDEO_Init();

    /* Set RESET/POWER button callback */
    SYS_SetResetCallback(__Sys_ResetCallback);
    SYS_SetPowerCallback(__Sys_PowerCallback);
}

static void _ExitApp()
{
    ExitGUIThreads();
    StopGX();
    ShutdownAudio();

    ResourceManager::DestroyInstance();

    UnmountNTFS();
    SDCard_deInit();
    USBDevice_deInit();
    mload_set_ES_ioctlv_vector(NULL);
    mload_close();
}

void Sys_Reboot(void)
{
    /* Restart console */
    _ExitApp();
    STM_RebootSystem();
}

#define ShutdownToDefault   0
#define ShutdownToIdle      1
#define ShutdownToStandby   2

static void _Sys_Shutdown(int SHUTDOWN_MODE)
{
    _ExitApp();
    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

    /* Poweroff console */
    if ((CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE && SHUTDOWN_MODE != ShutdownToStandby) || SHUTDOWN_MODE
            == ShutdownToIdle)
    {
        s32 ret;

        /* Set LED mode */
        ret = CONF_GetIdleLedMode();
        if (ret >= 0 && ret <= 2) STM_SetLedMode(ret);

        /* Shutdown to idle */
        STM_ShutdownToIdle();
    }
    else
    {
        /* Shutdown to standby */
        STM_ShutdownToStandby();
    }
}

void Sys_Shutdown(void)
{
    _Sys_Shutdown(ShutdownToDefault);
}

void Sys_ShutdownToIdel(void)
{
    _Sys_Shutdown(ShutdownToIdle);
}
void Sys_ShutdownToStandby(void)
{
    _Sys_Shutdown(ShutdownToStandby);
}

void Sys_LoadMenu(void)
{
    _ExitApp();
    /* Return to the Wii system menu */
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

void Sys_BackToLoader(void)
{

    if (hbcStubAvailable())
    {
        _ExitApp();
        exit(0);
    }
    // Channel Version
    Sys_LoadMenu();
}

bool Sys_IsHermes()
{
    return IOS_GetVersion() == 222 || IOS_GetVersion() == 223;
}

void ScreenShot()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];
    char buffer2[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    //USBLoader_GX_ScreenShot-Month_Day_Hour_Minute_Second_Year.png
    strftime(buffer, 80, "USBLoader_GX_ScreenShot-%b%d%H%M%S%y.png", timeinfo);
    sprintf(buffer2, "%s/config/%s", bootDevice, buffer);

    TakeScreenshot(buffer2);
}
