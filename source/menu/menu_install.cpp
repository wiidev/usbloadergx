#include "menus.h"
#include "usbloader/usbstorage2.h"
#include "usbloader/wbfs.h"
#include "usbloader/disc.h"
#include "usbloader/utils.h"
#include "usbloader/GameList.h"
#include "prompts/ProgressWindow.h"
#include "themes/CTheme.h"

float gamesize;

/****************************************************************************
 * MenuInstall
 ***************************************************************************/

int MenuInstall()
{
    gprintf("\nMenuInstall()");

    static struct discHdr headerdisc ATTRIBUTE_ALIGN( 32 );

    Disc_SetUSB(NULL);

    int ret, choice = 0;
    char name[200];

    ret = DiscWait(tr( "Insert Disk" ), tr( "Waiting..." ), tr( "Cancel" ), 0, 0);
    if (ret < 0)
    {
        WindowPrompt(tr( "Error reading Disc" ), 0, tr( "Back" ));
        return MENU_DISCLIST;
    }
    ret = Disc_Open();
    if (ret < 0)
    {
        WindowPrompt(tr( "Could not open Disc" ), 0, tr( "Back" ));
        return MENU_DISCLIST;
    }

    ret = Disc_IsWii();
    if (ret < 0)
    {
        choice = WindowPrompt(tr( "Not a Wii Disc" ), tr( "Insert a Wii Disc!" ), tr( "OK" ), tr( "Back" ));

        if (choice == 1)
            return MENU_INSTALL;
        else
            return MENU_DISCLIST;
    }

    Disc_ReadHeader(&headerdisc);
    snprintf(name, sizeof(name), "%s", headerdisc.title);

    ret = WBFS_CheckGame(headerdisc.id);
    if (ret)
    {
        WindowPrompt(tr( "Game is already installed:" ), name, tr( "Back" ));
        return MENU_DISCLIST;
    }

    f32 freespace, used;

    WBFS_DiskSpace(&used, &freespace);
    gamesize = WBFS_EstimeGameSize() / GB_SIZE;

    char gametxt[50];

    sprintf(gametxt, "%s : %.2fGB", name, gamesize);

    wiilight(1);
    choice = WindowPrompt(tr( "Continue to install game?" ), gametxt, tr( "OK" ), tr( "Cancel" ));

    if (choice == 1)
    {
        sprintf(gametxt, "%s", tr( "Installing game:" ));

        if (gamesize > freespace)
        {
            char errortxt[50];
            sprintf(errortxt, "%s: %.2fGB, %s: %.2fGB", tr( "Game Size" ), gamesize, tr( "Free Space" ), freespace);
            WindowPrompt(tr( "Not enough free space!" ), errortxt, tr( "OK" ));
            return MENU_DISCLIST;
        }
        else
        {
            USBStorage2_Watchdog(0);
            SetupGameInstallProgress(gametxt, name);
            ret = WBFS_AddGame();
            ProgressStop();
            USBStorage2_Watchdog(1);
            wiilight(0);
            if (ret != 0)
            {
                WindowPrompt(tr( "Install Error!" ), 0, tr( "Back" ));
                return MENU_DISCLIST;
            }
            else
            {
                gameList.ReadGameList(); //get the entries again
                gameList.FilterList();
                GuiSound * instsuccess = NULL;
                bgMusic->Pause();
                instsuccess = new GuiSound(success_ogg, success_ogg_size, Settings.sfxvolume);
                instsuccess->SetVolume(Settings.sfxvolume);
                instsuccess->SetLoop(0);
                instsuccess->Play();
                WindowPrompt(tr( "Successfully installed:" ), name, tr( "OK" ));
                instsuccess->Stop();
                delete instsuccess;
                bgMusic->Resume();
                return MENU_DISCLIST;
            }
        }
    }
    else
        return MENU_DISCLIST;

    //Turn off the WiiLight
    wiilight(0);

    return MENU_DISCLIST;
}
