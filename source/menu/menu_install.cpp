#include "menus.h"
#include "usbloader/usbstorage.h"
#include "usbloader/wbfs.h"
#include "usbloader/disc.h"
#include "usbloader/utils.h"
#include "usbloader/getentries.h"
#include "prompts/ProgressWindow.h"

float gamesize;

/****************************************************************************
 * MenuInstall
 ***************************************************************************/

int MenuInstall() {
	gprintf("\nMenuInstall()");

    int menu = MENU_NONE;
    static struct discHdr headerdisc ATTRIBUTE_ALIGN(32);

    Disc_SetUSB(NULL);

    int ret, choice = 0;
    char name[200];

    GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, Settings.sfxvolume);

    char imgPath[100];

    snprintf(imgPath, sizeof(imgPath), "%sbattery.png", CFG.theme_path);
    GuiImageData battery(imgPath, battery_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_bar.png", CFG.theme_path);
    GuiImageData batteryBar(imgPath, battery_bar_png);
	snprintf(imgPath, sizeof(imgPath), "%sbattery_red.png", CFG.theme_path);
    GuiImageData batteryRed(imgPath, battery_red_png);
    snprintf(imgPath, sizeof(imgPath), "%sbattery_bar_red.png", CFG.theme_path);
    GuiImageData batteryBarRed(imgPath, battery_bar_red_png);

    HaltGui();
    GuiWindow w(screenwidth, screenheight);

    mainWindow->Append(&w);

    ResumeGui();

    while (menu == MENU_NONE) {
        VIDEO_WaitVSync ();

        ret = DiscWait(tr("Insert Disk"),tr("Waiting..."),tr("Cancel"),0,0);
        if (ret < 0) {
            WindowPrompt (tr("Error reading Disc"),0,tr("Back"));
            menu = MENU_DISCLIST;
            break;
        }
        ret = Disc_Open();
        if (ret < 0) {
            WindowPrompt (tr("Could not open Disc"),0,tr("Back"));
            menu = MENU_DISCLIST;
            break;
        }

        ret = Disc_IsWii();
        if (ret < 0) {
            choice = WindowPrompt (tr("Not a Wii Disc"),tr("Insert a Wii Disc!"),tr("OK"),tr("Back"));

            if (choice == 1) {
                menu = MENU_INSTALL;
                break;
            } else
                menu = MENU_DISCLIST;
            break;
        }

        Disc_ReadHeader(&headerdisc);
        snprintf(name, sizeof(name), "%s", headerdisc.title);

        ret = WBFS_CheckGame(headerdisc.id);
        if (ret) {
            WindowPrompt (tr("Game is already installed:"),name,tr("Back"));
            menu = MENU_DISCLIST;
            break;
        }

        f32 freespace, used;

        WBFS_DiskSpace(&used, &freespace);
        gamesize = WBFS_EstimeGameSize()/GB_SIZE;

        char gametxt[50];

        sprintf(gametxt, "%s : %.2fGB", name, gamesize);

        wiilight(1);
        choice = WindowPrompt(tr("Continue to install game?"),gametxt,tr("OK"),tr("Cancel"));

        if (choice == 1) {

            sprintf(gametxt, "%s", tr("Installing game:"));

            if (gamesize > freespace) {
                char errortxt[50];
                sprintf(errortxt, "%s: %.2fGB, %s: %.2fGB",tr("Game Size"), gamesize, tr("Free Space"), freespace);
                WindowPrompt(tr("Not enough free space!"),errortxt,tr("OK"));
                menu = MENU_DISCLIST;
                break;
            } else {
                USBStorage_Watchdog(0);
                SetupGameInstallProgress(gametxt, name);
                ret = WBFS_AddGame();
                ProgressStop();
                USBStorage_Watchdog(1);
                wiilight(0);
                if (ret != 0) {
                    WindowPrompt(tr("Install Error!"),0,tr("Back"));
                    menu = MENU_DISCLIST;
                    break;
                } else {
                    __Menu_GetEntries(); //get the entries again
					GuiSound * instsuccess = NULL;
					bgMusic->Pause();
					instsuccess = new GuiSound(success_ogg, success_ogg_size, Settings.sfxvolume);
					instsuccess->SetVolume(Settings.sfxvolume);
					instsuccess->SetLoop(0);
					instsuccess->Play();
                    WindowPrompt (tr("Successfully installed:"),name,tr("OK"));
					instsuccess->Stop();
					delete instsuccess;
					bgMusic->Resume();
                    menu = MENU_DISCLIST;
                    break;
                }
            }
        } else {
            menu = MENU_DISCLIST;
            break;
        }
    }

    //Turn off the WiiLight
    wiilight(0);

    HaltGui();

    mainWindow->Remove(&w);
    ResumeGui();
    return menu;
}
