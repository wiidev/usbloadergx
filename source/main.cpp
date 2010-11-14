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
#include <di/di.h>
#include <sys/iosupport.h>

#include "libwiigui/gui.h"
#include "usbloader/wbfs.h"
#include "language/gettext.h"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "FreeTypeGX.h"
#include "FontSystem.h"
#include "video.h"
#include "audio.h"
#include "menu/menus.h"
#include "menu.h"
#include "input.h"
#include "filelist.h"
#include "FileOperations/fileops.h"
#include "main.h"
#include "fatmounter.h"
#include "sys.h"
#include "gecko.h"
#include "usbloader/partition_usbloader.h"
#include "usbloader/usbstorage2.h"
#include "memory/mem2.h"
#include "lstub.h"
#include "usbloader/usbstorage2.h"
#include "wad/nandtitle.h"
#include "system/IosLoader.h"
#include "GameBootProcess.h"

extern "C"
{
    extern void __exception_setreload(int t);
}

extern bool geckoinit;

PartList partitions;

int main(int argc, char *argv[])
{
    MEM2_init(48);
    setlocale(LC_ALL, "en.UTF-8");
    geckoinit = InitGecko();
    InitVideo();
    __exception_setreload(20);

    printf("\tStarting up\n");
    NandTitles.Get();

    //Let's use libogc sd/usb for config loading
    printf("\tInitialize sd card\n");
    SDCard_Init();
    printf("\tInitialize usb device\n");
    USBDevice_Init();

    gettextCleanUp();
    printf("\tLoading configuration...");
    Settings.Load();
    VIDEO_SetWidescreen(Settings.widescreen);
    printf("done\n");

    // Let's load the cIOS now
    if (IosLoader::LoadAppCios() < 0)
    {
        printf("\n\tWARNING!\n");
        printf("\tUSB Loader GX needs unstubbed cIOS 222 v4+ or 249 v9+\n\n");

        printf(
                "\tWe cannot determine the versions on your system,\n\tsince you have no patched ios 36 or 236 installed.\n");
        printf("\tTherefor, if loading of USB Loader GX fails, you\n\tprobably have installed the 4.2 update,\n");
        printf("\tand you should go figure out how to get some cios action going on\n\tin your Wii.\n");

        printf("\tERROR: No cIOS could be loaded. Exiting....\n");
        sleep(10);
        Sys_BackToLoader();
    }
    printf("\tLoaded cIOS = %u (Rev %u)\n", IOS_GetVersion(), IOS_GetRevision());

    //if a ID was passed via args copy it and try to boot it after the partition is mounted
    //its not really a headless mode.  more like hairless.
    if (argc > 1 && argv[1])
    {
        MountGamePartition(false);
        return BootGame(argv[1]);
    }

    //! Init the rest of the System
    Sys_Init();
    SetupPads();
    InitAudio();

    char *fontPath = NULL;
    asprintf(&fontPath, "%sfont.ttf", Settings.theme_path);
    SetupDefaultFont(fontPath);
    free(fontPath);

    MainMenu(MENU_DISCLIST);
    return 0;
}
