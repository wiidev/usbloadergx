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
#include "settings/CGameSettings.h"
#include "settings/CGameStatistics.h"
#include "themes/CTheme.h"
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
    void __exception_setreload(int t);
}

PartList partitions;

int main(int argc, char *argv[])
{
    MEM2_init(48);
    setlocale(LC_ALL, "en.UTF-8");
    InitVideo();
    InitGecko();
    __exception_setreload(20);

    printf("\tStarting up\n");
    NandTitles.Get();

    // Let's try loading some cIOS
    if (IosLoader::LoadAppCios() < 0)
    {
        printf("\n\tWARNING!\n");
        printf("\tUSB Loader GX needs unstubbed cIOS 222 v4+ or 249 v9+\n\n");

        printf("\tWe cannot determine the versions on your system,\n\tsince you have no patched ios 36 or 236 installed.\n");
        printf("\tTherefor, if loading of USB Loader GX fails, you\n\tprobably have installed the 4.2 update,\n");
        printf("\tand you should go figure out how to get some cios action going on\n\tin your Wii.\n");

        printf("\tERROR: No cIOS could be loaded. Exiting....\n");
        sleep(10);
        Sys_BackToLoader();
    }

    //Let's use libogc sd/usb for config loading
    printf("\tInitialize sd card...%s\n", SDCard_Init() < 0 ? "failed" : "done");
    printf("\tInitialize usb device...%s\n", USBDevice_Init_Loop() < 0 ? "failed" : "done");

    //Load configurations
    printf("\tLoading config...%s\n", Settings.Load() ? "done" : "failed");
    printf("\tLoading language...%s\n", Settings.LoadLanguage(Settings.language_path, CONSOLE_DEFAULT) ? "done" : "failed");
    printf("\tLoading game settings...%s\n", GameSettings.Load(Settings.ConfigPath) ? "done" : "failed");
    printf("\tLoading game statistics...%s\n", GameStatistics.Load(Settings.ConfigPath) ? "done" : "failed");
    printf("\tLoading theme...%s\n", Theme.Load(Settings.theme_path) ? "done" : "failed (using default)");
    printf("\tLoading font system...%s\n", SetupDefaultFont(Settings.theme_path) ? "done" : "failed (using default)");

    VIDEO_SetWidescreen(Settings.widescreen);

    if(Settings.cios != IOS_GetVersion())
    {
        // Unmount fat before reloading IOS.
        SDCard_deInit();
        USBDevice_deInit();

        // Loading now the cios setup in the settings
        IosLoader::LoadAppCios();
        printf("\tLoaded cIOS = %u (Rev %u)\n", IOS_GetVersion(), IOS_GetRevision());

        // Remount devices after reloading IOS.
        SDCard_Init();
        USBDevice_Init_Loop();
    }

    //if a ID was passed via args copy it and try to boot it after the partition is mounted
    //its not really a headless mode.  more like hairless.
    if (argc > 1 && argv[1])
    {
        MountGamePartition(false);
        return BootGame(argv[1]);
    }

    //! Now we startup the GUI so no need for console prints. Output only to gecko.
    USBGeckoOutput();

    //! Init the rest of the System
    Sys_Init();
    SetupPads();
    InitAudio();

    MainMenu(MENU_DISCLIST);
    return 0;
}
