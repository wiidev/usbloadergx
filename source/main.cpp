/****************************************************************************
 * USB Loader GX Team
 *
 * Main loadup of the application
 *
 * libwiigui
 * Tantric 2009
 ***************************************************************************/

#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <ogcsys.h>
#include <unistd.h>
#include <locale.h>
#include <wiiuse/wpad.h>
//#include <debug.h>
extern "C"
{
    extern void __exception_setreload( int t );
}


#include <di/di.h>
#include <sys/iosupport.h>

#include "libwiigui/gui.h"
#include "usbloader/wbfs.h"
#include "settings/cfg.h"
#include "language/gettext.h"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "FreeTypeGX.h"
#include "FontSystem.h"
#include "video.h"
#include "audio.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "listfiles.h"
#include "main.h"
#include "fatmounter.h"
#include "sys.h"
#include "wpad.h"
#include "fat.h"
#include "gecko.h"
#include "svnrev.h"
#include "usbloader/partition_usbloader.h"
#include "usbloader/usbstorage2.h"
#include "memory/mem2.h"
#include "lstub.h"
#include "usbloader/usbstorage2.h"
#include "wad/nandtitle.h"

extern bool geckoinit;
extern char headlessID[8];

NandTitle titles;
PartList partitions;


int main( int argc, char *argv[] )
{
    MEM2_init( 48 );
    setlocale( LC_ALL, "en.UTF-8" );
    geckoinit = InitGecko();
    InitVideo();
    __exception_setreload( 20 );

    printf( "\tStarting up\n" );
    titles.Get();

    bool bootDevice_found = false;
    if ( argc >= 1 )
    {
        if ( !strncasecmp( argv[0], "usb:/", 5 ) )
        {
            strcpy( bootDevice, "USB:" );
            bootDevice_found = true;
        }
        else if ( !strncasecmp( argv[0], "sd:/", 4 ) )
            bootDevice_found = true;
    }

    //Let's use libogc sd/usb for config loading
    printf( "\tInitialize sd card\n" );
    SDCard_Init();
    printf( "\tInitialize usb device\n" );
    USBDevice_Init();

    if ( !bootDevice_found )
    {
        printf( "\tSearch for configuration file\n" );
        //try USB
        //left in all the dol and elf files in this check in case this is the first time running the app and they dont have the config
        if ( checkfile( ( char* ) "USB:/config/GXglobal.cfg" ) || ( checkfile( ( char* ) "USB:/apps/usbloader_gx/boot.elf" ) )
                || checkfile( ( char* ) "USB:/apps/usbloadergx/boot.dol" ) || ( checkfile( ( char* ) "USB:/apps/usbloadergx/boot.elf" ) )
                || checkfile( ( char* ) "USB:/apps/usbloader_gx/boot.dol" ) )
            strcpy( bootDevice, "USB:" );

        printf( "\tConfiguration file is on %s\n", bootDevice );
    }

    gettextCleanUp();
    printf( "\tLoading configuration..." );
    CFG_Load();
    VIDEO_SetWidescreen(CFG.widescreen);
    printf( "done\n" );

    SDCard_deInit();// unmount SD for reloading IOS
    USBDevice_deInit();// unmount USB for reloading IOS
    USBStorage2_Deinit();

    printf( "\tCheck for an existing cIOS\n" );
    CheckForCIOS();

    // Let's load the cIOS now
    if ( LoadAppCIOS() < 0 )
    {
	printf( "\tERROR: No cIOS could be loaded. Exiting....\n" );
        sleep( 5 );
        Sys_BackToLoader();
    }

    printf( "\tLoaded cIOS = %u (Rev %u)\n", IOS_GetVersion(), IOS_GetRevision() );

    printf( "\tWaiting for USB:\n" );
    if ( MountWBFS() < 0 )
    {
        printf( "ERROR: No WBFS drive mounted.\n" );
        sleep( 5 );
        exit( 0 );
    }

    //if a ID was passed via args copy it and try to boot it after the partition is mounted
    //its not really a headless mode.  more like hairless.
    if ( argc > 1 && argv[1] )
    {
        if ( strlen( argv[1] ) == 6 )
            strncpy( headlessID, argv[1], sizeof( headlessID ) );
    }

    //! Init the rest of the System

    Sys_Init();
    SetupPads();
    InitAudio();

    char *fontPath = NULL;
    asprintf( &fontPath, "%sfont.ttf", CFG.theme_path );
    SetupDefaultFont( fontPath );
    free( fontPath );

    gprintf( "\n\tEnd of Main()" );
    InitGUIThreads();
    MainMenu( MENU_CHECK );
    return 0;
}
