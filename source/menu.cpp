/****************************************************************************
 * USB Loader GX Team
 *
 * libwiigui Template
 * by Tantric 2009
 *
 * menu.cpp
 * Menu flow routines - handles all menu logic
 ***************************************************************************/
#include <unistd.h>

#include "libwiigui/gui.h"
#include "homebrewboot/BootHomebrew.h"
#include "homebrewboot/HomebrewBrowse.h"
#include "prompts/ProgressWindow.h"
#include "menu/GameBrowseMenu.hpp"
#include "menu/menus.h"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "network/networkops.h"
#include "patches/patchcode.h"
#include "settings/Settings.h"
#include "settings/CGameSettings.h"
#include "themes/CTheme.h"
#include "themes/ThemeMenu.h"
#include "themes/ThemeDownloader.h"
#include "usbloader/disc.h"
#include "usbloader/GameList.h"
#include "usbloader/MountGamePartition.h"
#include "mload/mload_modules.h"
#include "xml/xml.h"
#include "audio.h"
#include "gecko.h"
#include "menu.h"
#include "sys.h"
#include "wpad.h"
#include "settings/newtitles.h"
#include "patches/fst.h"
#include "usbloader/frag.h"
#include "usbloader/wbfs.h"
#include "wad/nandtitle.h"

/*** Variables that are also used extern ***/
GuiWindow * mainWindow = NULL;
GuiImageData * pointer[4];
GuiImage * bgImg = NULL;
GuiImageData * background = NULL;
GuiBGM * bgMusic = NULL;
GuiSound *btnSoundClick = NULL;
GuiSound *btnSoundClick2 = NULL;
GuiSound *btnSoundOver = NULL;

int currentMenu = 0;
u8 mountMethod = 0;

static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static bool ExitRequested = false;

/****************************************************************************
 * ResumeGui
 *
 * Signals the GUI thread to start, and resumes the thread. This is called
 * after finishing the removal/insertion of new elements, and after initial
 * GUI setup.
 ***************************************************************************/
void ResumeGui()
{
    guiHalt = false;
    LWP_ResumeThread(guithread);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
void HaltGui()
{
    if (guiHalt) return;
    guiHalt = true;

    // wait for thread to finish
    while (!LWP_ThreadIsSuspended(guithread))
        usleep(100);
}

/****************************************************************************
 * UpdateGUI
 *
 * Primary thread to allow GUI to respond to state changes, and draws GUI
 ***************************************************************************/
static void * UpdateGUI(void *arg)
{
    int i;

    while (!ExitRequested)
    {
        if (guiHalt)
        {
            LWP_SuspendThread(guithread);
            continue;
        }

        mainWindow->Draw();
        if (Settings.tooltips == ON && Theme::ShowTooltips && mainWindow->GetState() != STATE_DISABLED) mainWindow->DrawTooltip();

        for (i = 3; i >= 0; i--)
        {
            if (userInput[i].wpad.ir.valid)
            {
                Menu_DrawImg(userInput[i].wpad.ir.x - 48, userInput[i].wpad.ir.y - 48, 200.0, 96, 96,
                        pointer[i]->GetImage(), userInput[i].wpad.ir.angle, Settings.widescreen ? 0.8 : 1, 1, 255, 0,
                        0, 0, 0, 0, 0, 0, 0);
            }
        }

        Menu_Render();

        UpdatePads();

        for (i = 0; i < 4; i++)
            mainWindow->Update(&userInput[i]);

        if (bgMusic) bgMusic->UpdateState();
	}

    for (i = 5; i < 255; i += 10)
    {
        mainWindow->Draw();
        Menu_DrawRectangle(0, 0, screenwidth, screenheight, (GXColor) {0, 0, 0, i}, 1);
        Menu_Render();
    }

    mainWindow->RemoveAll();
    ShutoffRumble();

    return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void InitGUIThreads()
{
    ExitRequested = false;

    if(guithread == LWP_THREAD_NULL)
        LWP_CreateThread(&guithread, UpdateGUI, NULL, NULL, 65536, LWP_PRIO_HIGHEST);
}

void ExitGUIThreads()
{
    ExitRequested = true;

    if(guithread != LWP_THREAD_NULL)
    {
        ResumeGui();
        LWP_JoinThread(guithread, NULL);
        guithread = LWP_THREAD_NULL;
    }
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
int MainMenu(int menu)
{
    currentMenu = menu;

    InitGUIThreads();

    InitProgressThread();
    InitNetworkThread();

    if (Settings.autonetwork)
        ResumeNetworkThread();

    btnSoundClick = new GuiSound(Resources::GetFile("button_click.wav"), Resources::GetFileSize("button_click.wav"), Settings.sfxvolume);
    btnSoundClick2 = new GuiSound(Resources::GetFile("button_click2.wav"), Resources::GetFileSize("button_click2.wav"), Settings.sfxvolume);
    btnSoundOver = new GuiSound(Resources::GetFile("button_over.wav"), Resources::GetFileSize("button_over.wav"), Settings.sfxvolume);

    pointer[0] = Resources::GetImageData("player1_point.png");
    pointer[1] = Resources::GetImageData("player2_point.png");
    pointer[2] = Resources::GetImageData("player3_point.png");
    pointer[3] = Resources::GetImageData("player4_point.png");

    mainWindow = new GuiWindow(screenwidth, screenheight);

    background = Resources::GetImageData(Settings.widescreen ? "wbackground.png" : "background.png");

    bgImg = new GuiImage(background);
    mainWindow->Append(bgImg);

    ResumeGui();

    bgMusic = new GuiBGM(Resources::GetFile("bg_music.ogg"), Resources::GetFileSize("bg_music.ogg"), Settings.volume);
    bgMusic->SetLoop(Settings.musicloopmode); //loop music
    bgMusic->Load(Settings.ogg_path);
    bgMusic->Play();

    MountGamePartition();

    while (currentMenu != MENU_EXIT)
    {
        bgMusic->SetVolume(Settings.volume);

        switch (currentMenu)
        {
            case MENU_INSTALL:
                currentMenu = MenuInstall();
                break;
            case MENU_SETTINGS:
                currentMenu = MenuSettings();
                break;
            case MENU_THEMEMENU:
                currentMenu = ThemeMenu::Run();
                break;
            case MENU_THEMEDOWNLOADER:
                currentMenu = ThemeDownloader::Run();
                break;
            case MENU_HOMEBREWBROWSE:
                currentMenu = MenuHomebrewBrowse();
                break;
            case MENU_DISCLIST:
            default: // unrecognized menu
                currentMenu = GameBrowseMenu::Execute();
                break;
        }
    }

    //! THIS SHOULD NEVER HAPPEN ANYMORE
    ExitApp();

    return -1;
}
