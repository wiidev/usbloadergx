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
    extern void __exception_setreload(int t);
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
#include "wad/title.h"
#include "usbloader/partition_usbloader.h"
#include "usbloader/usbstorage2.h"
#include "memory/mem2.h"
#include "lstub.h"
#include "usbloader/usbstorage2.h"

extern bool geckoinit;
extern bool textVideoInit;
extern char headlessID[8];

/* Constants */
#define CONSOLE_XCOORD      260
#define CONSOLE_YCOORD      115
#define CONSOLE_WIDTH       340
#define CONSOLE_HEIGHT      218

PartList partitions;
u8 dbvideo =0;

int main(int argc, char *argv[])
{
    MEM2_init(48);
    InitVideo();
    setlocale(LC_ALL, "en.UTF-8");
    geckoinit = InitGecko();
    __exception_setreload(20);

    printf("\n\tStarting up");

    bool bootDevice_found=false;
    if (argc >= 1)
    {
        if (!strncasecmp(argv[0], "usb:/", 5))
        {
            strcpy(bootDevice, "USB:");
            bootDevice_found = true;
        } else if (!strncasecmp(argv[0], "sd:/", 4))
        bootDevice_found = true;
    }

    //Let's use libogc sd/usb for config loading
    printf("\n\tInitialize sd card");
    SDCard_Init();
    printf("\n\tInitialize usb device");
    USBDevice_Init();

    if (!bootDevice_found)
    {
        printf("\n\tSearch for configuration file");
        //try USB
        //left in all the dol and elf files in this check in case this is the first time running the app and they dont have the config
        if (checkfile((char*) "USB:/config/GXglobal.cfg") || (checkfile((char*) "USB:/apps/usbloader_gx/boot.elf"))
            || checkfile((char*) "USB:/apps/usbloadergx/boot.dol") || (checkfile((char*) "USB:/apps/usbloadergx/boot.elf"))
            || checkfile((char*) "USB:/apps/usbloader_gx/boot.dol"))
            strcpy(bootDevice, "USB:");

        printf("\n\tConfiguration file is on %s", bootDevice);
    }

    gettextCleanUp();
    printf("\n\tLoading configuration...");
    CFG_Load();
    printf("done");

    SDCard_deInit();// unmount SD for reloading IOS
    USBDevice_deInit();// unmount USB for reloading IOS
    USBStorage2_Deinit();

    // This part is added, because we need a identify patched ios
    //! pune please replace this with your magic patch functions - Dimok
    if (IOS_ReloadIOSsafe(236) < 0)
        IOS_ReloadIOSsafe(36);

    printf("\n\tCheck for an existing cIOS");
    CheckForCIOS();

    // Let's load the cIOS now
    if(LoadAppCIOS() < 0)
    {
        printf("\n\tERROR: No cIOS could be loaded. Exiting....");
        sleep(5);
        Sys_BackToLoader();
    }

    printf("\n\tLoaded cIOS = %u (Rev %u)",IOS_GetVersion(), IOS_GetRevision());

    printf("\n\tWaiting for USB: ");
    if (MountWBFS() < 0)
    {
        printf("\nERROR: No WBFS drive mounted.");
        sleep(5);
        exit(0);
    }

    //if a ID was passed via args copy it and try to boot it after the partition is mounted
    //its not really a headless mode.  more like hairless.
    if (argc > 1 && argv[1])
    {
        if (strlen(argv[1])==6)
            strncpy(headlessID, argv[1], sizeof(headlessID));
    }

    //! Init the rest of the System
    Sys_Init();
    SetupPads();
    InitAudio();

    char *fontPath = NULL;
    asprintf(&fontPath, "%sfont.ttf", CFG.theme_path);
    SetupDefaultFont(fontPath);
    free(fontPath);

    gprintf("\n\tEnd of Main()");
    InitGUIThreads();
    MainMenu(MENU_CHECK);
    return 0;
}
