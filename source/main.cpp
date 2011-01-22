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
#include "usbloader/GameBooter.hpp"
#include "StartUpProcess.h"
#include "sys.h"

extern "C"
{
    extern s32 MagicPatches(s32);
    void __exception_setreload(int t);
}

static int QuickGameBoot(const char * gameID)
{
    DeviceHandler::Instance()->MountAll();
    Settings.Load();

    MountGamePartition(false);
    return GameBooter::BootGame(gameID);
}

int main(int argc, char *argv[])
{
    if(IOS_GetVersion() != 58)
        IOS_ReloadIOS(58);

    MEM2_init(48);
    __exception_setreload(20);
    MagicPatches(1);
    InitVideo();
    InitGecko();
    USBGeckoOutput();
    NandTitles.Get();
    setlocale(LC_ALL, "en.UTF-8");

    if(argc > 1 && argv[1])
        return QuickGameBoot(argv[1]);

	StartUpProcess::Run();

    MainMenu(MENU_DISCLIST);
    return 0;
}
