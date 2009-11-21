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
#include "network/networkops.h"
#include "patches/patchcode.h"
#include "settings/Settings.h"
#include "settings/cfg.h"
#include "themes/Theme_Downloader.h"
#include "usbloader/disc.h"
#include "wad/title.h"
#include "xml/xml.h"
#include "audio.h"
#include "gecko.h"
#include "menu.h"
#include "sys.h"
#include "wpad.h"
#include "settings/newtitles.h"

/*** Variables that are also used extern ***/
GuiWindow * mainWindow = NULL;
GuiImageData * pointer[4];
GuiImage * bgImg = NULL;
GuiImageData * background = NULL;
GuiSound * bgMusic = NULL;
GuiSound *btnClick2 = NULL;

struct discHdr *dvdheader = NULL;
float gamesize;
int currentMenu;
u8 mountMethod=0;

char game_partition[6];
bool load_from_fat;

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
extern struct discHdr * gameList;
extern FreeTypeGX *fontClock;
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
void
ResumeGui() {
    guiHalt = false;
    LWP_ResumeThread (guithread);
}

/****************************************************************************
 * HaltGui
 *
 * Signals the GUI thread to stop, and waits for GUI thread to stop
 * This is necessary whenever removing/inserting new elements into the GUI.
 * This eliminates the possibility that the GUI is in the middle of accessing
 * an element that is being changed.
 ***************************************************************************/
void
HaltGui() {
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
static void * UpdateGUI (void *arg) {
    while (1) {
        if (guiHalt) {
            LWP_SuspendThread(guithread);
        } else {
            if (!ExitRequested) {
                mainWindow->Draw();
                if (Settings.tooltips == TooltipsOn && THEME.show_tooltip != 0 && mainWindow->GetState() != STATE_DISABLED)
                    mainWindow->DrawTooltip();

#ifdef HW_RVL
                for (int i=3; i >= 0; i--) { // so that player 1's cursor appears on top!
                    if (userInput[i].wpad.ir.valid)
                        Menu_DrawImg(userInput[i].wpad.ir.x-48, userInput[i].wpad.ir.y-48, 200.0,
                                     96, 96, pointer[i]->GetImage(), userInput[i].wpad.ir.angle, CFG.widescreen? 0.8 : 1, 1, 255,0,0,0,0,0,0,0,0);
                    if (Settings.rumble == RumbleOn) {
                        DoRumble(i);
                    }
                }
#endif

                Menu_Render();

                for (int i=0; i < 4; i++)
                    mainWindow->Update(&userInput[i]);


            } else {
                for (int a = 5; a < 255; a += 10) {
                    mainWindow->Draw();
                    Menu_DrawRectangle(0,0,screenwidth,screenheight,(GXColor) {0, 0, 0, a},1);
                    Menu_Render();
                }
                mainWindow->RemoveAll();
                ShutoffRumble();
                return 0;
            }
        }

        switch (Settings.screensaver) {
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
    return NULL;
}

/****************************************************************************
 * InitGUIThread
 *
 * Startup GUI threads
 ***************************************************************************/
void InitGUIThreads() {
    LWP_CreateThread(&guithread, UpdateGUI, NULL, NULL, 0, 75);
    InitProgressThread();
    InitNetworkThread();

    if (Settings.autonetwork)
        ResumeNetworkThread();
}

void ExitGUIThreads() {
    ExitRequested = 1;
    LWP_JoinThread(guithread, NULL);
    guithread = LWP_THREAD_NULL;
}

/****************************************************************************
 * LoadCoverImage
 ***************************************************************************/
GuiImageData *LoadCoverImage(struct discHdr *header, bool Prefere3D, bool noCover)
{
	if(!header) return NULL;
	GuiImageData *Cover = NULL;
	char ID[4];
	char IDfull[7];
	char Path[100];
	bool flag = Prefere3D;


	snprintf(ID, sizeof(ID), "%c%c%c", header->id[0], header->id[1], header->id[2]);
	snprintf(IDfull, sizeof(IDfull), "%s%c%c%c", ID, header->id[3], header->id[4], header->id[5]);

	for(int i=0; i<2; ++i)
	{
		char *coverPath = flag ? Settings.covers_path : Settings.covers2d_path; flag = !flag;
		//Load full id image
		snprintf(Path, sizeof(Path), "%s%s.png", coverPath, IDfull);
		delete Cover; Cover = new(std::nothrow) GuiImageData(Path, NULL);
		//Load short id image
		if (!Cover || !Cover->GetImage())
		{
			snprintf(Path, sizeof(Path), "%s%s.png", coverPath, ID);
			delete Cover; Cover = new(std::nothrow) GuiImageData(Path, NULL);
		}
		if(Cover && Cover->GetImage())
			break;
	}
	//Load no image
	if (noCover && (!Cover || !Cover->GetImage()))
	{
		flag = Prefere3D;
		for(int i=0; i<2; ++i)
		{
			const char *nocoverPath = (flag ? "%snoimage.png" : "%snoimage2d.png"); flag = !flag;
			snprintf(Path, sizeof(Path), nocoverPath, CFG.theme_path);
			delete Cover; Cover = new(std::nothrow) GuiImageData(Path, (Prefere3D ? nocover_png : nocoverFlat_png));
			if (Cover && Cover->GetImage())
				break;
		}
	}
	if(Cover && !Cover->GetImage())
	{
		delete Cover;
		Cover = NULL;
	}
	return Cover;
}

/****************************************************************************
 * MainMenu
 ***************************************************************************/
int MainMenu(int menu) {

    currentMenu = menu;
    char imgPath[100];

#ifdef HW_RVL
    snprintf(imgPath, sizeof(imgPath), "%splayer1_point.png", CFG.theme_path);
    pointer[0] = new GuiImageData(imgPath, player1_point_png);
    snprintf(imgPath, sizeof(imgPath), "%splayer2_point.png", CFG.theme_path);
    pointer[1] = new GuiImageData(imgPath, player2_point_png);
    snprintf(imgPath, sizeof(imgPath), "%splayer3_point.png", CFG.theme_path);
    pointer[2] = new GuiImageData(imgPath, player3_point_png);
    snprintf(imgPath, sizeof(imgPath), "%splayer4_point.png", CFG.theme_path);
    pointer[3] = new GuiImageData(imgPath, player4_point_png);
#endif

    mainWindow = new GuiWindow(screenwidth, screenheight);

    if (CFG.widescreen)
        snprintf(imgPath, sizeof(imgPath), "%swbackground.png", CFG.theme_path);
    else
        snprintf(imgPath, sizeof(imgPath), "%sbackground.png", CFG.theme_path);

    background = new GuiImageData(imgPath, CFG.widescreen? wbackground_png : background_png);

    bgImg = new GuiImage(background);
    mainWindow->Append(bgImg);

    ResumeGui();

    bgMusic = new GuiSound(bg_music_ogg, bg_music_ogg_size, Settings.volume);
    bgMusic->SetLoop(1); //loop music
    // startup music
    if (strcmp("", Settings.oggload_path) && strcmp("notset", Settings.ogg_path)) {
        bgMusic->Load(Settings.ogg_path);
    }
	bgMusic->Play();

    while (currentMenu != MENU_EXIT) {
        bgMusic->SetVolume(Settings.volume);

        switch (currentMenu) {
        case MENU_CHECK:
            currentMenu = MenuCheck();
            break;
        case MENU_FORMAT:
            currentMenu = MenuFormat();
            break;
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
            currentMenu = MenuDiscList();
            break;
        default: // unrecognized menu
            currentMenu = MenuCheck();
            break;
        }
    }

	// MemInfoPrompt();
	//for testing
	/*if (mountMethod)
	{
		char tmp[30];
		sprintf(tmp,"boot method -->   %i",mountMethod);
		WindowPrompt(0,tmp,0,0,0,0,100);
	}
	*/
	gprintf("\nExiting main GUI");

	CloseXMLDatabase();
	NewTitles::DestroyInstance();
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
	delete fontClock;
	delete fontSystem;
	ShutdownAudio();
    StopGX();
	gettextCleanUp();

	if (mountMethod==3)
	{
			struct discHdr *header = &gameList[gameSelected];
			char tmp[20];
			u32 tid;
			sprintf(tmp,"%c%c%c%c",header->id[0],header->id[1],header->id[2],header->id[3]);
			memcpy(&tid, tmp, 4);
			gprintf("\nBooting title %016llx",TITLE_ID((header->id[5]=='1'?0x00010001:0x00010002),tid));
			WII_Initialize();
			WII_LaunchTitle(TITLE_ID((header->id[5]=='1'?0x00010001:0x00010002),tid));
	}
	if (mountMethod==2)
	{
		gprintf("\nLoading BC for GameCube");
		WII_Initialize();
		WII_LaunchTitle(0x0000000100000100ULL);
	}

    if (boothomebrew == 1) {
		gprintf("\nBootHomebrew");
        BootHomebrew(Settings.selected_homebrew);
    } else if (boothomebrew == 2) {
		gprintf("\nBootHomebrewFromMenu");
        BootHomebrewFromMem();
    } else {
        int ret = 0;
        struct discHdr *header = (mountMethod?dvdheader:&gameList[gameSelected]);

        struct Game_CFG* game_cfg = CFG_get_game_opt(header->id);
		
        if (game_cfg) {
            videoChoice = game_cfg->video;
            languageChoice = game_cfg->language;
            ocarinaChoice = game_cfg->ocarina;
            viChoice = game_cfg->vipatch;
            fix002 = game_cfg->errorfix002;
            iosChoice = game_cfg->ios;
            countrystrings = game_cfg->patchcountrystrings;
			if (!altdoldefault) {
				alternatedol = game_cfg->loadalternatedol;
				alternatedoloffset = game_cfg->alternatedolstart;
			}
            reloadblock = game_cfg->iosreloadblock;
        } else {
            videoChoice = Settings.video;
            languageChoice = Settings.language;
            ocarinaChoice = Settings.ocarina;
            viChoice = Settings.vpatch;
            if (Settings.cios == ios222) {
                iosChoice = i222;
            } else {
                iosChoice = i249;
            }
            fix002 = Settings.error002;
            countrystrings = Settings.patchcountrystrings;
			if (!altdoldefault) {
				alternatedol = off;
				alternatedoloffset = 0;
			}
            reloadblock = off;
        }
		int ios2;

		switch (iosChoice) {
		case i249:
			ios2 = 249;
			break;

		case i222:
			ios2 = 222;
			break;

		case i223:
			ios2 = 223;
			break;

		default:
			ios2 = 249;
			break;
		}

		// When the selected ios is 249, and you're loading from FAT, reset ios to 222
		if (load_from_fat && ios2 == 249) {
			ios2 = 222;
		}
        bool onlinefix = !load_from_fat && ShutdownWC24();

		// You cannot reload ios when loading from fat
        if (IOS_GetVersion() != ios2 || onlinefix) {
            ret = Sys_ChangeIos(ios2);
            if (ret < 0) {
                Sys_ChangeIos(249);
            }
        }
		if (!mountMethod)
		{
			ret = Disc_SetUSB(header->id);
			if (ret < 0) Sys_BackToLoader();
			gprintf("\n\tUSB set to game");
		}
		else {
			gprintf("\n\tUSB not set, loading DVD");
		}
        ret = Disc_Open();

        if (ret < 0) Sys_BackToLoader();

        if (gameList){
			free(gameList);
		}
		if(dvdheader)
			delete dvdheader;

		if (reloadblock == on && Sys_IsHermes()) {
            patch_cios_data();
			if (!load_from_fat) {
				mload_close();
			}
        }

        u8 errorfixer002 = 0;
        switch (fix002) {
        case on:
            errorfixer002 = 1;
            break;
        case off:
            errorfixer002 = 0;
            break;
        case anti:
            errorfixer002 = 2;
            break;
        }

        switch (languageChoice) {
        case ConsoleLangDefault:
            configbytes[0] = 0xCD;
            break;

        case jap:
            configbytes[0] = 0x00;
            break;

        case eng:
            configbytes[0] = 0x01;
            break;

        case ger:
            configbytes[0] = 0x02;
            break;

        case fren:
            configbytes[0] = 0x03;
            break;

        case esp:
            configbytes[0] = 0x04;
            break;

        case it:
            configbytes[0] = 0x05;
            break;

        case dut:
            configbytes[0] = 0x06;
            break;

        case schin:
            configbytes[0] = 0x07;
            break;

        case tchin:
            configbytes[0] = 0x08;
            break;

        case kor:
            configbytes[0] = 0x09;
            break;
            //wenn nicht genau klar ist welches
        default:
            configbytes[0] = 0xCD;
            break;
        }

        u8 videoselected = 0;

        switch (videoChoice) {
        case discdefault:
            videoselected = 0;
            break;

        case pal50:
            videoselected = 1;
            break;

        case pal60:
            videoselected = 2;
            break;

        case ntsc:
            videoselected = 3;
            break;

        case systemdefault:
            videoselected = 4;
            break;

        case patch:
            videoselected = 5;
            break;

        default:
            videoselected = 0;
            break;
        }

        u32 cheat = 0;
        switch (ocarinaChoice) {
        case on:
            cheat = 1;
            break;

        case off:
            cheat = 0;
            break;

        default:
            cheat = 1;
            break;
        }

        u8 vipatch = 0;
        switch (viChoice) {
        case on:
            vipatch = 1;
            break;

        case off:
            vipatch = 0;
            break;

        default:
            vipatch = 0;
            break;
        }
		gprintf("\n\tDisc_wiiBoot");

        ret = Disc_WiiBoot(videoselected, cheat, vipatch, countrystrings, errorfixer002, alternatedol, alternatedoloffset);
        if (ret < 0) {
            Sys_LoadMenu();
        }
		
		printf("Returning entry point: 0x%0x\n", ret);
    }

    return 0;
}
