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
#include "menu/menus.h"
#include "mload/mload.h"
#include "mload/mload_modules.h"
#include "network/networkops.h"
#include "patches/patchcode.h"
#include "settings/Settings.h"
#include "settings/CGameSettings.h"
#include "themes/CTheme.h"
#include "themes/Theme_Downloader.h"
#include "usbloader/disc.h"
#include "usbloader/GameList.h"
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
#include "GameBootProcess.h"

/*** Variables that are also used extern ***/
GuiWindow * mainWindow = NULL;
GuiImageData * pointer[4];
GuiImage * bgImg = NULL;
GuiImageData * background = NULL;
GuiBGM * bgMusic = NULL;
GuiSound *btnClick2 = NULL;

int currentMenu;
u8 mountMethod = 0;

char game_partition[6];
int load_from_fs;

/*** Variables used only in the menus ***/
GuiText * GameIDTxt = NULL;
GuiText * GameRegionTxt = NULL;
GuiImage * coverImg = NULL;
GuiImageData * cover = NULL;
bool altdoldefault = true;

static lwp_t guithread = LWP_THREAD_NULL;
static bool guiHalt = true;
static int ExitRequested = 0;

/*** Extern variables ***/
extern u8 shutdown;
extern u8 reset;
extern s32 gameSelected, gameStart;
extern u8 boothomebrew;

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
        usleep(50);
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
        if (Settings.tooltips == ON && Theme.show_tooltip != 0 && mainWindow->GetState() != STATE_DISABLED) mainWindow->DrawTooltip();

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

        switch (Settings.screensaver)
        {
            case 1:
                WPad_SetIdleTime(180);
                break;
            case 2:
                WPad_SetIdleTime(300);
                break;
            case 3:
                WPad_SetIdleTime(600);
                break;
            case 4:
                WPad_SetIdleTime(1200);
                break;
            case 5:
                WPad_SetIdleTime(1800);
                break;
            case 6:
                WPad_SetIdleTime(3600);
                break;
        }
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
    LWP_CreateThread(&guithread, UpdateGUI, NULL, NULL, 65536, LWP_PRIO_HIGHEST);
    InitProgressThread();
    InitNetworkThread();

    if (Settings.autonetwork) ResumeNetworkThread();
}

void ExitGUIThreads()
{
    ExitRequested = 1;
    LWP_JoinThread(guithread, NULL);
    guithread = LWP_THREAD_NULL;
}

/****************************************************************************
 * LoadCoverImage
 ***************************************************************************/
GuiImageData *LoadCoverImage(struct discHdr *header, bool Prefere3D, bool noCover)
{
    if (!header) return NULL;
    GuiImageData *Cover = NULL;
    char ID[4];
    char IDfull[7];
    char Path[100];
    bool flag = Prefere3D;

    snprintf(ID, sizeof(ID), "%c%c%c", header->id[0], header->id[1], header->id[2]);
    snprintf(IDfull, sizeof(IDfull), "%s%c%c%c", ID, header->id[3], header->id[4], header->id[5]);

    for (int i = 0; i < 2; ++i)
    {
        char *coverPath = flag ? Settings.covers_path : Settings.covers2d_path;
        flag = !flag;
        //Load full id image
        snprintf(Path, sizeof(Path), "%s%s.png", coverPath, IDfull);
        delete Cover;
        Cover = new (std::nothrow) GuiImageData(Path);
        //Load short id image
        if (!Cover || !Cover->GetImage())
        {
            snprintf(Path, sizeof(Path), "%s%s.png", coverPath, ID);
            delete Cover;
            Cover = new (std::nothrow) GuiImageData(Path);
        }
        if (Cover && Cover->GetImage()) break;
    }
    //Load no image
    if (noCover && (!Cover || !Cover->GetImage()))
    {
        flag = Prefere3D;
        for (int i = 0; i < 2; ++i)
        {
            flag = !flag;
            delete Cover;
            Cover = Resources::GetImageData(Prefere3D ? "nocover.png" : "nocoverFlat.png");
            if (Cover && Cover->GetImage()) break;
        }
    }
    if (Cover && !Cover->GetImage())
    {
        delete Cover;
        Cover = NULL;
    }
    return Cover;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
int MainMenu(int menu)
{
    currentMenu = menu;

    pointer[0] = Resources::GetImageData("player1_point.png");
    pointer[1] = Resources::GetImageData("player2_point.png");
    pointer[2] = Resources::GetImageData("player3_point.png");
    pointer[3] = Resources::GetImageData("player4_point.png");

    mainWindow = new GuiWindow(screenwidth, screenheight);

    background = Resources::GetImageData(Settings.widescreen ? "wbackground.png" : "background.png");

    bgImg = new GuiImage(background);
    mainWindow->Append(bgImg);

    ResumeGui();

    bgMusic = new GuiBGM(bg_music_ogg, bg_music_ogg_size, Settings.volume);
    bgMusic->SetLoop(Settings.musicloopmode); //loop music
    bgMusic->Load(Settings.ogg_path);
    bgMusic->Play();

    MountGamePartition();

    while (currentMenu != MENU_EXIT)
    {
        bgMusic->SetVolume(Settings.volume);
        //      gprintf("Current menu: %d\n", currentMenu);

        switch (currentMenu)
        {
            case MENU_INSTALL:
                currentMenu = MenuInstall();
                break;
            case MENU_SETTINGS:
                currentMenu = MenuSettings();
                break;
            case MENU_THEMEDOWNLOADER:
                currentMenu = Theme_Downloader();
                break;
            case MENU_HOMEBREWBROWSE:
                currentMenu = MenuHomebrewBrowse();
                break;
            case MENU_DISCLIST:
            default: // unrecognized menu
                currentMenu = MenuDiscList();
                break;
        }
    }

    // MemInfoPrompt();
    gprintf("Exiting main GUI.  mountMethod = %d\n", mountMethod);

    CloseXMLDatabase();
    NewTitles::DestroyInstance();

	ResumeGui();
    ExitGUIThreads();

    bgMusic->Stop();
    delete bgMusic;
    delete background;
    delete bgImg;
    delete mainWindow;
    for (int i = 0; i < 4; i++)
        delete pointer[i];
    delete GameRegionTxt;
    delete GameIDTxt;
    delete cover;
    delete coverImg;
    delete fontSystem;
    ShutdownAudio();
    StopGX();
    gettextCleanUp();

    if (mountMethod == 3)
    {
        struct discHdr *header = gameList[gameSelected];
        char tmp[20];
        u32 tid;
        sprintf(tmp, "%c%c%c%c", header->id[0], header->id[1], header->id[2], header->id[3]);
        memcpy(&tid, tmp, 4);
        gprintf("\nBooting title %016llx", TITLE_ID( ( header->id[5] == '1' ? 0x00010001 : 0x00010002 ), tid ));
        WII_Initialize();
        WII_LaunchTitle(TITLE_ID( ( header->id[5] == '1' ? 0x00010001 : 0x00010002 ), tid ));
    }
    if (mountMethod == 2)
    {
        gprintf("\nLoading BC for GameCube");
        WII_Initialize();
        WII_LaunchTitle(0x0000000100000100ULL);
    }

    if (boothomebrew == 1)
    {
        gprintf("\nBootHomebrew");
        BootHomebrew(Settings.selected_homebrew);
        return 0;
    }
    else if (boothomebrew == 2)
    {
        gprintf("\nBootHomebrew from Menu");
        BootHomebrewFromMem();
        return 0;
    }

    return BootGame((const char *) gameList[gameSelected]->id);
}
