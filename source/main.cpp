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

#include "video.h"
#include "themes/CTheme.h"
#include "menu/menus.h"
#include "main.h"
#include "Controls/DeviceHandler.hpp"
#include "settings/CSettings.h"
#include "memory/mem2.h"
#include "wad/nandtitle.h"
#include "system/IosLoader.h"
#include "usbloader/MountGamePartition.h"
#include "StartUpProcess.h"
#include "GameBootProcess.h"
#include "sys.h"

extern "C"
{
    void __exception_setreload(int t);
}

static int QuickGameBoot(const char * gameID)
{
    //if a ID was passed via args copy it and try to boot it after the partition is mounted
    //its not really a headless mode.  more like hairless.
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

    DeviceHandler::Instance()->MountAll();
    Settings.Load();

    MountGamePartition(false);
    return BootGame(gameID);
}

int main(int argc, char *argv[])
{
    MEM2_init(48);
    __exception_setreload(20);
    InitVideo();
    InitGecko();
    NandTitles.Get();
    setlocale(LC_ALL, "en.UTF-8");

    if(argc > 1 && argv[1])
        return QuickGameBoot(argv[1]);

	StartUpProcess::Run();

    MainMenu(MENU_DISCLIST);
    return 0;
}
