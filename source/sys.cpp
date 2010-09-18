#include <stdio.h>
#include <ogcsys.h>
#include <unistd.h>

#include "usbloader/wdvd.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/disc.h"
#include "usbloader/wbfs.h"
#include "usbloader/partition_usbloader.h"
#include "mload/mload_modules.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "fatmounter.h"
#include "gecko.h"
#include "sys.h"
#include "wpad.h"
#include "lstub.h"
#include "wad/nandtitle.h"

extern char game_partition[6];
extern u8 load_from_fs;

//Wiilight stuff
static vu32 *_wiilight_reg = ( u32* )0xCD0000C0;
void wiilight( int enable )             // Toggle wiilight (thanks Bool for wiilight source)
{
    u32 val = ( *_wiilight_reg&~0x20 );
    if ( enable && Settings.wiilight ) val |= 0x20;
    *_wiilight_reg = val;
}

/* Variables */
u8 shutdown = 0;
u8 reset = 0;

void __Sys_ResetCallback( void )
{
    /* Reboot console */
    reset = 1;
}

void __Sys_PowerCallback( void )
{
    /* Poweroff console */
    shutdown = 1;
}

void Sys_Init( void )
{
    /* Initialize video subsytem */
    //VIDEO_Init();

    /* Set RESET/POWER button callback */
    SYS_SetResetCallback( __Sys_ResetCallback );
    SYS_SetPowerCallback( __Sys_PowerCallback );
}

static void _ExitApp()
{
    ExitGUIThreads();
    StopGX();
    ShutdownAudio();

    UnmountNTFS();
    SDCard_deInit();
    USBDevice_deInit();
    mload_set_ES_ioctlv_vector( NULL );
    mload_close();
}

void Sys_Reboot( void )
{
    /* Restart console */
    _ExitApp();
    STM_RebootSystem();
}

int Sys_ChangeIos( int ios )
{
    s32 prevIos = IOS_GetVersion();

    SDCard_deInit();
    USBDevice_deInit();

    WPAD_Flush( 0 );
    WPAD_Disconnect( 0 );
    WPAD_Shutdown();

    WDVD_Close();

    USBStorage2_Deinit();

    s32 ret = IOS_ReloadIOSsafe( ios );
    if ( ret < 0 )
    {
        ios = prevIos;
    }

    SDCard_Init();

    if ( ios == 222 || ios == 223 )
    {
        load_ehc_module();
    }
    USBDevice_Init();

    PAD_Init();
    Wpad_Init();
    WPAD_SetDataFormat( WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR );
    WPAD_SetVRes( WPAD_CHAN_ALL, screenwidth, screenheight );

    WBFS_Init( WBFS_DEVICE_USB );
    Disc_Init();

    if ( Sys_IsHermes() )
    {
        WBFS_OpenNamed( ( char * ) &game_partition );
    }
    else
    {
        WBFS_Open();
    }

    return ret;
}

int Sys_IosReload( int IOS )
{
    s32 ret = -1;

    //shutdown SD and USB before IOS Reload in DiscWait
    SDCard_deInit();
    USBDevice_deInit();

    WPAD_Flush( 0 );
    WPAD_Disconnect( 0 );
    WPAD_Shutdown();

    WDVD_Close();

    USBStorage2_Deinit();

    if ( IOS == 249 || IOS == 222 || IOS == 223 )
    {
        for ( int i = 0; i < 10; i++ )
        {
            ret = IOS_ReloadIOSsafe( IOS );
            if ( ret < 0 ) return ret;
            if ( IOS == 222 || IOS == 223 ) load_ehc_module();
            ret = WBFS_Init( WBFS_DEVICE_USB );
            if ( !( ret < 0 ) ) break;
            sleep( 1 );
            USBStorage2_Deinit();
        }
        if ( ret >= 0 )
        {
            ret = Disc_Init();
            if ( ret >= 0 )
            {
                ret = WBFS_Open();
            }
        }
        else Sys_BackToLoader();
    }

    PAD_Init();
    Wpad_Init();
    WPAD_SetDataFormat( WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR );
    WPAD_SetVRes( WPAD_CHAN_ALL, screenwidth, screenheight );
    //reinitialize SD and USB
    SDCard_Init();
    USBDevice_Init();

    return ret;
}

#define ShutdownToDefault   0
#define ShutdownToIdle      1
#define ShutdownToStandby   2

static void _Sys_Shutdown( int SHUTDOWN_MODE )
{
    _ExitApp();
    WPAD_Flush( 0 );
    WPAD_Disconnect( 0 );
    WPAD_Shutdown();

    /* Poweroff console */
    if ( ( CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE &&  SHUTDOWN_MODE != ShutdownToStandby ) || SHUTDOWN_MODE == ShutdownToIdle )
    {
        s32 ret;

        /* Set LED mode */
        ret = CONF_GetIdleLedMode();
        if ( ret >= 0 && ret <= 2 )
            STM_SetLedMode( ret );

        /* Shutdown to idle */
        STM_ShutdownToIdle();
    }
    else
    {
        /* Shutdown to standby */
        STM_ShutdownToStandby();
    }
}
void Sys_Shutdown( void )
{
    _Sys_Shutdown( ShutdownToDefault );
}
void Sys_ShutdownToIdel( void )
{
    _Sys_Shutdown( ShutdownToIdle );
}
void Sys_ShutdownToStandby( void )
{
    _Sys_Shutdown( ShutdownToStandby );
}

void Sys_LoadMenu( void )
{
    _ExitApp();
    /* Return to the Wii system menu */
    SYS_ResetSystem( SYS_RETURNTOMENU, 0, 0 );
}

void Sys_BackToLoader( void )
{

    if ( hbcStubAvailable() )
    {
        _ExitApp();
        exit( 0 );
    }
    // Channel Version
    Sys_LoadMenu();
}

bool Sys_IsHermes()
{
    return IOS_GetVersion() == 222 || IOS_GetVersion() == 223;
}

#include "prompts/PromptWindows.h"

void ShowMemInfo()
{
    char buf[255];
    struct mallinfo mymallinfo = mallinfo();
    sprintf( ( char * ) &buf, "Total: %d, Used: %d, Can be freed: %d", mymallinfo.arena / 1024, mymallinfo.uordblks / 1024, mymallinfo.keepcost / 1024 );
    WindowPrompt( "Mem info", ( char * ) &buf, "OK" );
}

s32 IOS_ReloadIOSsafe( int ios )
{
    if ( ios == 222 )
    {
        s32 ios222rev = titles.VersionOf( 0x1000000deULL );
        if ( !ios222rev )
            return -2;
        if ( ios222rev != 4 && ios222rev != 5 && ios222rev != 65535 )
            return -2;
    }
    else if ( ios == 223 )
    {
        s32 ios223rev = titles.VersionOf( 0x1000000dfULL );
        if ( !ios223rev )
            return -2;
        if ( ios223rev != 4 && ios223rev != 5 && ios223rev != 65535 )
            return -2;
    }
    else if ( ios == 249 )
    {
        s32 ios249rev = titles.VersionOf( 0x1000000f9ULL );
        if ( !ios249rev )
            return -2;
        if ( ios249rev < 9 || ios249rev == 65280 )
            return -2;
    }
    else if ( ios == 250 )
    {
        s32 ios250rev = titles.VersionOf( 0x1000000faULL );
        if ( !ios250rev )
            return -2;
        if ( ios250rev < 9 || ios250rev == 65280 )
            return -2;
    }

    s32 r = IOS_ReloadIOS( ios );
    if ( r >= 0 )
    {
        WII_Initialize();
    }
    return r;
}

s32 CheckForCIOS()
{
    gprintf( "\tChecking for stub IOS\n" );
    s32 ret = 1;
    s32 ios222rev = titles.VersionOf( 0x00000001000000deULL );
    s32 ios249rev = titles.VersionOf( 0x00000001000000f9ULL );

    u8 ios222OK = !( ios222rev < 4 || ios222rev == 65280 );
    u8 ios249OK = !( ios249rev < 9 || ios249rev == 65280 );

    gprintf( "222ok: %u\t 249ok: %u\n", ios222OK, ios249OK );
    return 1;

    //if we don't like either of the cIOS then scram
    //if( !((ios222rev >= 4 && ios222rev < 65280) || (ios249rev >=9 && ios249rev < 65280)))
    if ( ios222rev < 4 || ios222rev == 65280 || ios249rev < 9 || ios249rev == 65280 )
    {
        printf( "\x1b[2J" );
        if ( ( ios222rev < 0 && ios222rev != WII_EINSTALL ) && ( ios249rev < 0 && ios249rev != WII_EINSTALL ) )
        {
            printf( "\n\n\n\tWARNING!" );
            printf( "\n\tUSB Loader GX needs unstubbed cIOS 222 v4 or 249 v9+" );
            printf( "\n\n\tWe cannot determine the versions on your system,\n\tsince you have no patched ios 36 or 236 installed." );
            printf( "\n\tTherefor, if loading of USB Loader GX fails, you\n\tprobably have installed the 4.2 update," );
            printf( "\n\tand you should go figure out how to get some cios action going on\n\tin your Wii." );
            printf( "\n\n\tThis message will show every time." );
            sleep( 5 );
        }
        else
        {
            printf( "\n\n\n\tERROR!" );
            printf( "\n\tUSB Loader GX needs unstubbed cIOS 222 v4 or 249 v9+" );
            printf( "\n\n\tI found \n\t\t222 = %d%s", ios222rev, ios222rev == 65280 ? " (Stubbed by 4.2 update)" : "" );
            printf( "\n\t\t249 = %d%s", ios249rev, ios249rev == 65280 ? " (Stubbed by 4.2 update)" : "" );
            printf( "\n\n\tGo figure out how to get some cIOS action going on\n\tin your Wii and come back and see me." );

            sleep( 15 );
            printf( "\n\n\tBye" );

            USBDevice_deInit();
            exit( 0 );
        }
    }

    return ret;
}

int LoadAppCIOS()
{
    s32 ret = 1;
    /* Load Custom IOS */
    SDCard_deInit();// unmount SD for reloading IOS
    USBDevice_deInit();// unmount USB for reloading IOS
    USBStorage2_Deinit();

    if ( Settings.cios == ios222 && IOS_GetVersion() != 222 )
    {
        printf( "\n\tReloading IOS to config setting (222)..." );
        ret = IOS_ReloadIOSsafe( 222 );
        printf( "%d", ret );
        if ( ret < 0 )
        {
            Settings.cios = ios249;
            IOS_ReloadIOSsafe( 249 );
        }
    }

    if ( ( Settings.cios == ios249  && IOS_GetVersion() != 249 )
            || ( Settings.cios == ios250 && IOS_GetVersion() != 250 ) )
    {
        printf( "\n\tReloading IOS to config setting (%d)...", ( Settings.cios == ios249 ) ? 249 : 250 );
        ret = IOS_ReloadIOSsafe( ( Settings.cios == ios249 ) ? 249 : 250 );
        printf( "%d", ret );
        if ( ret < 0 )
        {
            Settings.cios = ios222;
            ret = IOS_ReloadIOSsafe( 222 );
        }
    }

    SDCard_Init();
    if ( IOS_GetVersion() == 222 )
        load_ehc_module();

    USBDevice_Init();

    return ret;
}


#include <time.h>

void ScreenShot()
{
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [80];
    char buffer2 [80];

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    //USBLoader_GX_ScreenShot-Month_Day_Hour_Minute_Second_Year.png
    strftime ( buffer, 80, "USBLoader_GX_ScreenShot-%b%d%H%M%S%y.png", timeinfo );
    sprintf( buffer2, "%s/config/%s", bootDevice, buffer );

    TakeScreenshot( buffer2 );
}
